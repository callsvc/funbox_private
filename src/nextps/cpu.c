#include <assert.h>
#include <string.h>
#include <stdckdint.h>

#include <types.h>
#include <cpu.h>
#include <stdio.h>

cpu_t * cpu_create(bus_t *bus) {
    cpu_t * cpu = fb_malloc(sizeof(cpu_t));
    cpu->gateway = bus;
    return cpu;
}

uint32_t cpu_read32(const cpu_t *cpu, const uint32_t address) {
    return bus_read(cpu->gateway, address);
}


#define REG32_W(cpu, reg, value)\
    if (reg)\
        cpu->regs[reg] = value
#define REG32_R(cpu, reg)\
    cpu->regs[reg]
void cpu_reset(cpu_t *cpu) {
    for (size_t i = 0; i < count_of(cpu->regs); i++) {
        REG32_W(cpu, i, 0xBADA55);
    }
    cop0_reset(&cpu->cop0);

    cpu->regs[0] = 0; // HW to 0
    cpu->exec_count = 0;
    cpu->pc = 0xBFC00000;

    // reseting the load delay slot for now, next instruction is 0
    memset(cpu->load_slot, 0, sizeof(cpu->load_slot));
    cpu->maskint = 0;
}

// load upper immediate
void op_lui(cpu_t *cpu, const uint32_t op) {
    const uint16_t imm = op & 0xFFFF;
    REG32_W(cpu, op >> 16 & 0x1F, imm << 16);

}
// or immediate
void op_ori(cpu_t *cpu, const uint32_t op) {
    const uint16_t imm = op & 0xFFFF;
    if ((op >> 16 & 0x1F) == (op >> 21 & 0x1F)) {
        REG32_W(cpu, op >> 16 & 0x1F, REG32_R(cpu, op >> 16 & 0x1F) | imm);
    } else {
        REG32_W(cpu,op >> 16 & 0x1F, (uint32_t)imm | REG32_R(cpu, op >> 21 & 0x1F));
    }

#if 1
    if (cpu->pc == 0xBFC00008)
        assert(cpu->regs[8] == 0x0013243f);
#endif
}

// store word
void op_sw(const cpu_t *cpu, const uint32_t op) {
    if (cpu->cop0.regs[12] & 0x10000) {
        // the “isolate cache” bit
        printf("the cache is being initialized\n");
        return;
    }

    const int16_t offset = op & 0xFFFF;
    const uint32_t value = REG32_R(cpu, op >> 16 & 0x1F);

    bus_write(cpu->gateway, REG32_R(cpu, op >> 21 & 0x1F) + offset, value);

}

void op_addiu(cpu_t *cpu, const uint32_t op) {
    const int16_t imm = op & 0xFFFF;
    REG32_W(cpu, op >> 16 & 0x1F, (int32_t)REG32_R(cpu, op >> 21 & 0x1F) + imm);
}

void op_addi(cpu_t *cpu, const uint32_t op) {
    const int16_t imm = op & 0xFFFF;

    uint32_t result = 0;
    if (ckd_add(&result, (int32_t)REG32_R(cpu, op >> 21 & 0x1F), imm)) {
    }
    REG32_W(cpu, op >> 16 & 0x1F, result);
}

void op_sltu(cpu_t *cpu, const uint32_t op) {
    REG32_W(cpu, op >> 11 & 0x1F, REG32_R(cpu, op >> 21 & 0x1F) < REG32_R(cpu, op >> 16 & 0x1F) );
}

void cpu_setbranch(cpu_t *cpu, const uint32_t next_pc) {
    cpu->delay_slot = cpu->pc;
    cpu->pc = next_pc;
}
uint32_t cpu_getpc(cpu_t *cpu) {
    uint32_t pc = cpu->pc;
    if (cpu->delay_slot) {
        pc = cpu->delay_slot;
        cpu->delay_slot = 0;
    } else {
        cpu->pc += 4;
    }
    return pc;
}
void op_j(cpu_t *cpu, const uint32_t op) {
    // to branch within the current 256 MB aligned region
    const uint32_t clear_pc = cpu->pc & 0xF0000000;
    cpu_setbranch(cpu, clear_pc | op << 2);
}

void op_or(cpu_t *cpu, const uint32_t op) {
    REG32_W(cpu, op >> 11 & 0x1F, REG32_R(cpu, op >> 21 & 0x1F) | REG32_R(cpu, op >> 16 & 0x1F));
}

void op_cop0_mtc(cpu_t *cpu, const uint32_t op) {
    cop0_mtc(&cpu->cop0, op >> 11 & 0x1F, REG32_R(cpu, op >> 16 & 0x1F));
}

void op_bne(cpu_t *cpu, const uint32_t op) {
    const uint32_t var = REG32_R(cpu, op >> 16 & 0x1F);
    const uint32_t test = REG32_R(cpu, op >> 21 & 0x1F);

    if (var != test)
        cpu_setbranch(cpu, (int32_t)cpu->pc + (int16_t)((op & 0xFFFF) << 2));
}

void op_lw(cpu_t *cpu, const uint32_t op) {
    if (cpu->cop0.regs[12] & 0x10000)
        return;

    int32_t offset = op & 0xFFFF;
    offset += REG32_R(cpu, op >> 21 & 0x1F);

    cpu->load_slot[0] = op >> 16 & 0x1F;
    cpu->load_slot[1] = bus_read(cpu->gateway, offset);

    cpu->load_slot[2]++;
}

void special_opcodes(cpu_t *cpu, const uint32_t op) {
    switch (op & 0x3F) {
        case 0x25:
            op_ori(cpu, op); break;
        case 0x2B:
            op_sltu(cpu, op); break;
        default:
    }
}

void cpu_run(cpu_t *cpu) {
    const uint32_t fetched = cpu_read32(cpu, cpu_getpc(cpu));
    cpu->exec_count++;

    if (fetched == 0xA5200180) // “store halfword
        cpu->maskint = 1;

    if (cpu->maskint)
        return;

    if (!cpu->load_slot[2] && cpu->load_slot[0] && cpu->load_slot[1]) {
        // timer for the delay slot
        REG32_W(cpu, cpu->load_slot[0], cpu->load_slot[1]);
        memset(cpu->load_slot, 0, sizeof(cpu->load_slot));
    } else if (cpu->load_slot[2]) {
        cpu->load_slot[2]--;
    }

    switch (fetched >> 26 & 0x3F) {
        case 0:
            if (fetched) special_opcodes(cpu, fetched);
            break;
        case 2:
            op_j(cpu, fetched); break;
        case 5:
            op_bne(cpu, fetched); break;
        case 8:
            op_addi(cpu, fetched); break;
        case 9:
            op_addiu(cpu, fetched); break;
        case 0xD:
            op_ori(cpu, fetched); break;
        case 0xF:
            op_lui(cpu, fetched); break;
        case 0x10:
            op_cop0_mtc(cpu, fetched); break;
        case 0x23:
            op_lw(cpu, fetched); break;
        case 0x2B:
            op_sw(cpu, fetched); break;
        default:
    }
}

void cpu_destroy(cpu_t *cpu) {
    fb_free(cpu);
}

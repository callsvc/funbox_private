#include <stdlib.h>
#include <ee/ee.h>

#include <string.h>

ee_t * ee_create(bridge_t *board) {
    ee_t * mips = funbox_malloc(sizeof(ee_t));
    mips->scratchpad = funbox_malloc(16 * 1024);
    mips->decfile = stdout;
    mips->bridge = board;
    return mips;
}

void setpc(ee_t *mips, const uint32_t pc) {
    if (!mips->delay_slot) {
        mips->pc = mips->next_pc ? mips->next_pc : pc;
        mips->next_pc = mips->pc + 4;
    } else {
        mips->pc = mips->delay_slot;
        mips->delay_slot = 0;
    }
}

void nop(const ee_t *mips) {
    fputs("nop\n", mips->decfile);
}

#define ee_set_low32(reg, index, value)\
    if (index)\
        reg[index].lanes[0].low = value
#define ee_get_low32(reg, index)\
    reg[index].lanes[0].low

void mfc0(ee_t *mips, const uint32_t dest, const uint32_t cpsrc) {
    ee_set_low32(mips->regs, dest, cop0_mfc0(&mips->cop0, cpsrc));

    fprintf(mips->decfile, "mfc0 %u, %s\n", dest, cop0_rname(dest));
}

void slti(ee_t *mips, const uint32_t dest, const uint32_t src, const uint32_t value) {
    ee_set_low32(mips->regs, dest, mips->regs[src].low != value);
    fprintf(mips->decfile, "slti %u, %u, %#x // # result = %lx\n", dest, src, value, mips->regs[dest].low);
}

void setbranch(ee_t *mips, const uint32_t pc_slot, const int32_t offset) {
    mips->delay_slot = pc_slot;
    mips->next_pc = mips->pc + 4 + offset;
}

void bne(ee_t *mips, const uint32_t first, const uint32_t sec, const int32_t offset) {
    const int32_t fixed_offset = offset << 2;
    if (ee_get_low32(mips->regs, first) != ee_get_low32(mips->regs, sec))
        setbranch(mips, mips->pc + 4, fixed_offset);

    fprintf(mips->decfile, "bne %u, %u, %d // $ branch was %s\n", first, sec, fixed_offset, mips->delay_slot ? "taked" : "not taked");
}

void lui(ee_t *mips, const uint32_t dest, const uint16_t imm) {
    ee_set_low32(mips->regs, dest, 0);
    ee_set_low32(mips->regs, dest, imm << 16);

    fprintf(mips->decfile, "lui %u, %#x\n", dest, imm);
}
void ori(ee_t *mips, const uint32_t dest, const uint32_t source, const uint16_t imm) {
    ee_set_low32(mips->regs, dest, ee_get_low32(mips->regs, source) | imm);
    fprintf(mips->decfile, "ori %u, %u, %#x\n", dest, source, imm);
}

void addi(ee_t *mips, const uint32_t dest, const uint32_t source, const int16_t imm) {
    ee_set_low32(mips->regs, dest, (int32_t)ee_get_low32(mips->regs, source) + imm);
    fprintf(mips->decfile, "addi %u, %u, %#x // (%d)\n", dest, source, imm, imm);
}
void addiu(ee_t *mips, const uint32_t dest, const uint32_t source, const uint16_t imm) {
    ee_set_low32(mips->regs, dest, ee_get_low32(mips->regs, source) + imm);
    fprintf(mips->decfile, "addiu %u, %u, %#x // (%u)\n", dest, source, imm, imm);
}

void jr(ee_t *mips, const uint32_t dest) {
    const uint32_t target_pc = ee_get_low32(mips->regs, dest);
    mips->next_pc = target_pc;
}

void mtc0(ee_t *mips, const uint32_t cpdest, const uint32_t source) {
    cop0_mtc0(&mips->cop0, cpdest, ee_get_low32(mips->regs, source));
    fprintf(mips->decfile, "mtc0 %u, %u\n", source, cpdest);
}

void ee_sigill(const ee_t *mips, const uint32_t opcode) {
    oskill("opcode %s not implemented, pc: %#x", to_binary(&opcode, 4), mips->pc);
}

void ee_quit(const ee_t * mips, const char *str, const uint32_t opcode, const bool cond) {
    if (!cond)
        return;
    fprintf(stderr, "%s\n", str);
    ee_sigill(mips, opcode);
}

void sync(const ee_t *mips) {
    fputs("sync\n", mips->decfile);
    fflush(mips->decfile);
}

uint32_t ee_solve_vaddr(const uint32_t addr, uint64_t *s) {
    const uint64_t segments[] = {*(uint64_t*)"KUSEG", *(uint64_t*)"KSEG0", *(uint64_t*)"KSEG1", *(uint64_t*)"KSSEG", *(uint64_t*)"KSEG3", *(uint64_t*)"SCRATCH"};

    uint64_t seg;
    uint64_t *k_seg = s ? s : &seg;
    *k_seg = 0;
    if (addr < 0x7FFFFFFF)
        *k_seg = segments[0];
    else if (addr >= 0x80000000 && addr < 0x9FFFFFFF)
        *k_seg = segments[1];
    else if (addr >= 0xA0000000 && addr < 0xBFFFFFFF)
        *k_seg = segments[2];
    else if (addr >= 0xC0000000 && addr < 0xDFFFFFFF)
        *k_seg = segments[3];
    else if (addr >= 0xE0000000 && addr < 0xFFFFFFFF)
        *k_seg = segments[4];

    // Scratchpad RAM
    if (addr >= 0x70000000 && addr < 0x70004000) {
        *k_seg = segments[5];
        return addr & 0x3FFF;
    }

    // Main RAM
    if (addr < 0x2000000)
        return addr & 0x1FFFFFF;
    if (addr >= 0x20000000 && addr < 0x40000000)
        return addr & 0x1FFFFFFF;
    if (addr >= 0x30100000 && addr < 0x32000000)
        return addr & 0x1EFFFFF;


    // BIOS
    static const uint32_t bios_mask[] = {0x1FC00000, 0x9FC00000, 0xBFC00000};
    for (size_t i = 0; i < count_of(bios_mask); i++)
        if (addr >= bios_mask[i] && addr < bios_mask[i] + 0x400000)
            return addr & 0x1FFFFFFF;

    if (*k_seg == segments[2] || *k_seg == segments[0])
        return addr & 0x1FFFFFFF;

    return addr;
}

uint32_t ee_read32(const ee_t * mips, const uint32_t addr) {
    uint64_t type;
    const uint32_t paddr = ee_solve_vaddr(addr, &type);
    if (unlikely(type == *(uint64_t*)"SCRATCH"))
        return *(uint32_t*)&mips->scratchpad[paddr];

    return bridge_read(mips->bridge, paddr);
}
void ee_write32(const ee_t * mips, const uint32_t addr, const uint32_t value) {
    uint64_t type;
    const uint32_t paddr = ee_solve_vaddr(addr, &type);
    if (unlikely(type == *(uint64_t*)"SCRATCH"))
        *(uint32_t*)&mips->scratchpad = value;
    else bridge_write(mips->bridge, paddr, value);
}

void sw(const ee_t * mips, const uint32_t source, const uint32_t base, const int16_t offset) {
    const uint32_t addr = ee_get_low32(mips->regs, base) + offset;

    const uint32_t value = ee_get_low32(mips->regs, source);
    ee_write32(mips, addr, value);
}

void special_list(ee_t *mips, const uint32_t inst) {
    ee_quit(mips, "not a special instruction", inst, inst >> 26 != 0);

    const uint32_t rs = inst >> 21 & 0x1F;
    const uint16_t imm = inst >> 6 & 0xFFFF;
    switch (inst & 0x3F) {
        case 8:
            if (!imm)
                jr(mips, rs);
            break;
        case 0xF:
            sync(mips); break; // NOP ALMOST
        default:
            mips->isinint = true;
            // ee_sigill(mips, inst);
    }
}

void cop0_list(ee_t *mips, const uint32_t inst) {
    ee_quit(mips, "not a cop0 instruction", inst, inst >> 26 != 0x10);

    const uint32_t fmt = inst >> 21 & 0x1F;
    const uint32_t rt = inst >> 16 & 0x1F;
    const uint32_t rd = inst >> 11 & 0x1F;
    switch (fmt) {
        case 0:
            mfc0(mips, rt, rd); break;
        case 0x4:
            mtc0(mips, rd, rt); break;
        default:
            ee_sigill(mips, inst);
    }

}

void ee_run(ee_t *mips) {
    if (mips->isinint)
        mips->isinint = mips->pc != 0xBFC00000;

    while (!mips->isinint) {
        const uint32_t fetched = ee_read32(mips, mips->pc);

        const static uint32_t trap_instructions[] = {
            0x42000002, // tlbwi
        };
        bool bypass = false;
        for (size_t i = 0; i < count_of(trap_instructions); i++)
            if ((bypass = fetched == trap_instructions[i]))
                setpc(mips, mips->pc + 4);
        if (bypass)
            continue;

        const uint32_t base = fetched >> 26 & 0x3F;
        const uint32_t rt = fetched >> 16 & 0x1F;
        const int32_t rs = fetched >> 21 & 0x1F;
        const uint16_t imm = fetched & 0xFFFF;

        fprintf(stdout, "(%x) read from (%x)\n", fetched, mips->pc);
        switch (base) {
            case 0:
                if (!fetched)
                    nop(mips);
                else special_list(mips, fetched);
                break;
            case 5:
                bne(mips, rs, rt, imm); break;
            case 8:
                addi(mips, rt, rs, imm); break;
            case 9:
                addiu(mips, rt, rs, imm); break;
            case 0x10:
                cop0_list(mips, fetched); break;
            case 0xA:
                slti(mips, rt, rs, imm); break;
            case 0xD:
                ori(mips, rt, rs, imm); break;
            case 0xF:
                lui(mips, rt, imm); break;
            case 0x2B:
                sw(mips, rt, rs, imm); break;
            default:
                ee_sigill(mips, fetched);
        }
        setpc(mips, mips->pc + 4);
    }
}

void ee_reset(ee_t *mips) {
    const uint32_t boot_entry = 0xBFC00000;

    memset(mips->regs, 0, sizeof(mips->regs));
    cop0_reset(&mips->cop0);
    setpc(mips, boot_entry);
}
void ee_destroy(ee_t *mips) {
    if (mips->decfile != stdout)
        fclose(mips->decfile);
    funbox_free(mips->scratchpad);
    funbox_free(mips);
}
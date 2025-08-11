#include <stdlib.h>

#include <dospc.h>
#include <pc8086.h>
void x86_flags_set(pc8086_t *cpu, const uint8_t v, const uint8_t b) { cpu->flags |= v << b; }
int32_t x86_flags_get(const pc8086_t *cpu, const uint8_t b) { return cpu->flags & (1 << b); }


pc8086_t * pc8086_create() {
    pc8086_t *pc = fb_malloc(sizeof(pc8086_t));
    return pc;

}
void pc8086_reset(pc8086_t *cpu) {
    cpu->cs = 0; cpu->ip = 0;
    cpu->ip = 0x7C00;
}

void jmp(pc8086_t *cpu, const uint8_t *m_ptr) {
    const int8_t rel = m_ptr[(cpu->cs << 4) + cpu->ip + 1];
    fprintf(stderr, "jmp %x (unk)\n", rel);
    cpu->ip += 2;
    cpu->ip += rel;
}
void cli(pc8086_t *cpu) {
    fprintf(stderr, "cli\n");
    x86_flags_set(cpu, 0, X86_FLAGS_INT);
    cpu->ip++;
}
void cld(pc8086_t *cpu) {
    fprintf(stderr, "cld\n");
    x86_flags_set(cpu, 0, X86_FLAGS_DIRECTION);
    cpu->ip++;
}

void xor(pc8086_t *cpu, const uint8_t *mptr) {
    const uint8_t modrm = mptr[cpu->cs << 4 | (cpu->ip + 1)];
    const uint8_t reg = modrm >> 3 & 0x07;
    const uint8_t rm = modrm & 0x07;

    if ((modrm >> 6 & 0x3) != 3)
        quit("xor with memory not implemented");

    fprintf(stderr, "xor r, r16\n");
    cpu->regs86[rm] ^= cpu->regs86[reg];
    x86_flags_set(cpu, cpu->regs86[rm] == 0, X86_FLAGS_ZERO);

    cpu->ip += 2;

}

static const char * s_list[] = {"es", "cs", "ss", "ds"};
void mov(pc8086_t *cpu, const uint8_t *mptr) {
    const uint8_t modrm = mptr[cpu->cs << 4 | (cpu->ip + 1)];
    const uint8_t reg = modrm >> 3 & 0x07;
    const uint8_t rm = modrm & 0x07;

    if ((modrm >> 6 & 0x03) != 0x03)
        return;

    switch (reg) {
        case 0: cpu->es = cpu->regs86[rm]; break;
        case 1: cpu->cs = cpu->regs86[rm]; break;
        case 2: cpu->ss = cpu->regs86[rm]; break;
        case 3: cpu->ds = cpu->regs86[rm]; break;
        default:
    }
    fprintf(stderr, "mov r, %s\n", s_list[reg]);
    cpu->ip += 2;
}


uint16_t * pc_cpu_getreg(pc8086_t *cpu, const uint8_t id) {
    switch (id) {
        case 0: return &cpu->ax;
        case 1: return &cpu->cx;
        case 2: return &cpu->dx;
        case 3: return &cpu->bx;
        case 4: return &cpu->sp;
        case 5: return &cpu->bp;
        case 6: return &cpu->si;
        case 7: return &cpu->di;
        default:
    }
    return NULL;
}

void mov16(pc8086_t *cpu, const uint8_t *mptr) {
    const uint8_t modrm = mptr[cpu->cs << 4 | (cpu->ip + 1)];
    const uint8_t reg = modrm >> 3 & 0x7;
    const uint8_t rm = modrm & 0x7;

    if ((modrm >> 6 & 0x03) != 0x03) {
        const uint16_t memory16 = *(uint16_t*)(&mptr[cpu->cs << 4 | (cpu->ip + 1)]);
        cpu->pc->pc_memory[memory16] = cpu->regs86[reg];

        cpu->ip += 3;
        return;
    }

    uint16_t *dest = pc_cpu_getreg(cpu, reg);
    const uint16_t *src = pc_cpu_getreg(cpu, rm);
    *dest = *src;
    cpu->ip += 2;
}

void movimm(pc8086_t *cpu, const uint8_t *mptr, const uint8_t id) {
    const uint16_t imm16 = *(uint16_t*)(&mptr[cpu->cs << 4 | (cpu->ip + 1)]);
    switch (id) {
        case 0: cpu->ax = imm16; break;
        case 1: cpu->cx = imm16; break;
        case 2: cpu->dx = imm16; break;
        case 3: cpu->bx = imm16; break;
        case 4: cpu->sp = imm16; break;
        case 5: cpu->bp = imm16; break;
        case 6: cpu->si = imm16; break;
        case 7: cpu->di = imm16; break;
        default:
    }
    fprintf(stderr, "mov %x, %x\n", id, imm16);
    cpu->ip += 3;
}

void movbytes(pc8086_t *cpu, const size_t size) {
    uint32_t src = ((uint32_t)cpu->ds << 4) + cpu->si;
    uint32_t dst = ((uint32_t)cpu->es << 4) + cpu->di;

    for (size_t i = 0; i < size; i++) {
        cpu->pc->pc_memory[dst++] = cpu->pc->pc_memory[src++];
    }
    const int32_t direction = x86_flags_get(cpu, X86_FLAGS_DIRECTION) ? -1 : 1;
    cpu->si += size * direction;
    cpu->di += size * direction;
}

void rep(pc8086_t *cpu, const uint8_t *mptr) {
    const uint8_t func = mptr[cpu->cs << 4 | (cpu->ip + 1)];
    while (cpu->cx) {
        switch (func) {
            case 0xA5: {
                const size_t size = func - 0xA4 + 1;
                movbytes(cpu, size);
            }
            default:
        }
        cpu->cx--;
    }
    cpu->ip += 2;
}

void jmpfar(pc8086_t *cpu, const uint8_t *mptr) {
    const uint16_t ip = *(uint16_t*)(&mptr[cpu->cs << 4 | (cpu->ip + 1)]);
    const uint16_t cs = *(uint16_t*)(&mptr[cpu->cs << 4 | (cpu->ip + 3)]);

    cpu->ip = ip;
    cpu->cs = cs;
}

void pc8086_run(pc8086_t *cpu) {
    const uint8_t *m = cpu->pc->pc_memory;
    const uint8_t opcode = m[cpu->cs << 4 | cpu->ip];
    switch (opcode) {
        case 0x31:
            xor(cpu, m); break;
        case 0xB8:
        case 0xB9:
        case 0xBA:
        case 0xBB:
        case 0xBC:
        case 0xBD:
        case 0xBE:
        case 0xBF:
            movimm(cpu, m, (opcode & 0x0F) - 8); break;
        case 0x89:
            mov16(cpu, m); break;
        case 0x8E:
            mov(cpu, m); break;
        case 0xEA:
            jmpfar(cpu, m); break;
        case 0xEB:
            jmp(cpu, m); break;
        case 0xFA:
            cli(cpu); break;
        case 0xFC:
            cld(cpu); break;
        case 0xF3:
            rep(cpu, m); break;
        default: cpu->int_ = true;
    }
}

void pc8086_destroy(pc8086_t *cpu) {
    fb_free(cpu);
}

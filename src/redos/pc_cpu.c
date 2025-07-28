#include <dospc.h>
#include <pc_cpu.h>
#include <stdlib.h>

void x86_flags_set(x86_flags_t *flags, const uint8_t v, const uint8_t b) { *flags |= v << b; }

pc_cpu_t * pc_cpu_create() {
    pc_cpu_t *pc = fb_malloc(sizeof(pc_cpu_t));
    return pc;

}
void pc_cpu_reset(pc_cpu_t *cpu) {
    cpu->cs = 0; cpu->ip = 0;
    cpu->ip = 0x7C00;
}

void jmp(pc_cpu_t *cpu, const uint8_t *m_ptr) {
    const int8_t rel = m_ptr[(cpu->cs << 4) + cpu->ip + 1];
    fprintf(stderr, "jmp %x (unk)\n", rel);
    cpu->ip += 2;
    cpu->ip += rel;
}
void cli(pc_cpu_t *cpu) {
    fprintf(stderr, "cli\n");
    x86_flags_set(&cpu->flags, 0, X86_FLAGS_BIT_INT);
    cpu->ip++;
}
void cld(pc_cpu_t *cpu) {
    fprintf(stderr, "cld\n");
    x86_flags_set(&cpu->flags, 0, X86_FLAGS_BIT_DIRECTION);
    cpu->ip++;
}

void pc_cpu_run(pc_cpu_t *cpu) {
    const uint8_t *m = cpu->pc->pc_memory;
    const uint8_t opcode = m[cpu->ip];
    switch (opcode) {
        case 0xEB:
            jmp(cpu, m); break;
        case 0xFA:
            cli(cpu); break;
        case 0xFC:
            cld(cpu); break;
        default: cpu->int_ = true;
    }
}

void pc_cpu_destroy(pc_cpu_t *cpu) {
    fb_free(cpu);
}

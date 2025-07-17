#include <ps.h>
#include <core/types.h>
cpu_t * cpu_create() {
    cpu_t * cpu = fb_malloc(sizeof(cpu_t));
    return cpu;
}

void cpu_reset(cpu_t *cpu) {
    cpu->pc = 0xBFC00000;
}

void cpu_destroy(cpu_t *cpu) {
    fb_free(cpu);
}

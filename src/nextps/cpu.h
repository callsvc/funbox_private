#pragma once

#include <ps.h>
#include <cop0.h>
typedef struct cpu {
    uint32_t pc;

    uint32_t regs[32];
    uint32_t hi, lo;
    uint32_t delay_slot;
    uint32_t load_slot[3];


    cop0_t cop0;
    uint64_t exec_count;

    bus_t *gateway;
    int32_t maskint;
} cpu_t;

cpu_t * cpu_create(bus_t *);
void cpu_reset(cpu_t*);

void cpu_run(cpu_t *);
void cpu_destroy(cpu_t*);
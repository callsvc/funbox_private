#pragma once
#include <stdint.h>

typedef struct cpu {
    uint32_t pc;
} cpu_t;

typedef struct bus {
    uint8_t *mips_memory;
} bus_t;

bus_t *bus_create();
void bus_destroy(bus_t *);

cpu_t * cpu_create();
void cpu_reset(cpu_t*);
void cpu_destroy(cpu_t*);
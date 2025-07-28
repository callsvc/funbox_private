#pragma once

#include <pc_cpu.h>
#include <disk.h>

typedef struct dospc {
    uint8_t pc_memory[1024 * 1024];
    uint32_t barrier;

    disk_t *disk_slot;
    pc_cpu_t *cpu;

    uint32_t cannary;
} dospc_t;

dospc_t* dospc_create();
void dospc_reset(dospc_t*);
void dospc_continue(const dospc_t*);
void dospc_destroy(dospc_t*);
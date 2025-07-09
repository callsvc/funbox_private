#pragma once
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>

#include <types.h>
#include <bridge.h>
#include <ee/cop0.h>


#define R5900_REGS_COUNT 32
typedef struct ee {
    uint32_t pc;
    uint32_t next_pc;
    FILE * decfile;

    bridge_t * bridge;
    uint8_t * scratchpad;

    ee_reg_t regs[R5900_REGS_COUNT];

    cop0_t cop0;
    uint32_t delay_slot;
    bool isinint;
} ee_t;

ee_t * ee_create(bridge_t*);
void ee_reset(ee_t*);
void ee_run(ee_t *);
void ee_destroy(ee_t *);
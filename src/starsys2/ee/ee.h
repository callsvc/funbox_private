#pragma once
#include <stdio.h>
#include <stdint.h>

#include <types.h>
#include <bridge.h>
#include <ee/cop0.h>

typedef enum ee_segment_type {
    ee_segment_kuseg,
    ee_segment_kseg0,
    ee_segment_kseg1,
    ee_segment_ksseg,
    ee_segment_kseg3,
    ee_segment_scratchpad
} ee_segment_type_e;

#define R5900_REGS_COUNT 32
typedef struct ee {
    uint32_t pc;
    uint32_t next_pc;
    FILE * decfile;

    bridge_t * bridge;
    uint8_t * scratchpad;
    size_t *cycles;

    ee_reg_t regs[R5900_REGS_COUNT];

    cop0_t cop0;
    uint32_t delay_slot;
    bool isinint;
} ee_t;

void ee_skip_cycles(const ee_t*);

ee_t * ee_create(bridge_t*);
void ee_reset(ee_t*);
void ee_run(ee_t *, size_t *);
void ee_destroy(ee_t *);
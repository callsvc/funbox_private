#pragma once
#include <stdint.h>

#define COP0_REGS_COUNT 32

#define COP_REG_PRID 15
#define COP_REG_CONFIG 16

typedef struct cop0 {
    uint32_t regs[COP0_REGS_COUNT];

    uint32_t prid;
    uint32_t config;
} cop0_t;

void cop0_reset(cop0_t*);

uint32_t cop0_mfc0(const cop0_t*, uint32_t );
void cop0_mtc0(cop0_t *, uint32_t, uint32_t);
const char * cop0_rname(uint32_t);

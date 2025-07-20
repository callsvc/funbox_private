#pragma once
#include <stdint.h>

typedef struct cop0 {
    uint32_t regs[64];
} cop0_t;

void cop0_reset(cop0_t*);
void cop0_mtc(cop0_t*, uint32_t, uint32_t);

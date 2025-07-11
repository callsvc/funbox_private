#pragma once
#include <stdint.h>
#include <core/types.h>

typedef struct ee_reg_half {
    uint32_t low;
    uint32_t high;
} ee_reg_half_t;

typedef struct ee_reg {
    union {
        struct {
            uint64_t low, high;
        };
        ee_reg_half_t lanes[2];
    };
} ee_reg_t;

typedef struct dev_ticks {
    size_t gain;
    size_t acc;
} dev_ticks_t;

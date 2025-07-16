#pragma once
#include <stdint.h>

#include <algo/list.h>
typedef struct arm_neon {
    uint64_t lane0, lane1;
} arm64_neon_t;

typedef struct jit_cfg_block {
    uint64_t start_pc;
    uint64_t end_pc;

    uint8_t *code;

    list_t *nested_blocks;
    list_t *irs;
    bool has_entrypoint;
} jit_cfg_block_t;

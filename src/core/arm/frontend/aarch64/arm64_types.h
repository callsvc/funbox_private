#pragma once
#include <stdint.h>

#include <arm/dynrec.h>
#include <algo/list.h>

#define ARM64_PC_REGID 0x80

#define NOP2_IR_OPCODE 0

typedef struct dynrec_frontend_arm64 {
    dynrec_frontend_t funcslist;
    dynrec_t * jit_gen;
} dynrec_frontend_arm64_t;

dynrec_frontend_arm64_t * dynrec_frontend_arm64_create(dynrec_t *jit);
void dynrec_frontend_arm64_destroy(dynrec_frontend_arm64_t *);

typedef struct arm64_instruction {
    const char *name;

    void (*irgen)(list_t*);
} arm64_instruction_t;

void arm64_lookup_table(arm64_instruction_t *ids, uint32_t inst);
void arm64_generate_irs(const dynrec_core_t *, jit_cfg_block_t *);


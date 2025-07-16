#pragma once
#include <stdint.h>

#include <algo/vector.h>
typedef struct dynrec dynrec_t;
typedef struct th_context {

    vector_t *regs;

    uint64_t pc_reg;

} th_context_t;

th_context_t *th_context_create(const dynrec_t *);
void th_context_destroy(th_context_t *);
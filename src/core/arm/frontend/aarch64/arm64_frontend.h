#pragma once

#include <arm/dynrec.h>

typedef struct dynrec_frontend_arm64 {
    dynrec_frontend_t funcs64;
    dynrec_t * parent;
} dynrec_frontend_arm64_t;

dynrec_frontend_arm64_t * dynrec_frontend_arm64_create(dynrec_t *jit);
void dynrec_frontend_arm64_destroy(dynrec_frontend_arm64_t *);
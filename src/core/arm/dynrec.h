#pragma once
#include <pthread.h>
#include <stdint.h>

#include <algo/robin.h>
#include <algo/list.h>
#include <arm/th_context.h>
#include <arm/types.h>
typedef enum dynrec_cpu_type {
    dynrec_type_armv8a = 0,
    dynrec_type_aarch64 = 0,
    dynrec_type_armv4t = 1,
    dynrec_type_arm7tdmi = 1,
} dynrec_cpu_type_e;

typedef struct dynrec_core {
    th_context_t * thc;

    dynrec_t *jit;
} dynrec_core_t;

typedef struct dynrec_frontend {
    size_t (*get_gprs_count)();
    dynrec_cpu_type_e (*get_type)(const struct dynrec_frontend*);
    void (*generate_irs)(const dynrec_core_t*, jit_cfg_block_t*);
    uint64_t (*read_gpr)(const th_context_t*, uint64_t);
    uint64_t (*get_pc)(const th_context_t*);
} dynrec_frontend_t;

typedef struct dynrec {
    dynrec_frontend_t *frontend_ctx;
    list_t *cores_list;

    pthread_mutex_t monitor_mutex;

    int32_t int_enabled;
    uint8_t *memory;
    size_t memory_size;

    robin_map_t *flow_cfg_blocks;
} dynrec_t;

dynrec_t * dynrec_create(dynrec_cpu_type_e type);

dynrec_core_t * dynrec_enablecore(dynrec_t*);
void dynrec_disablecore(const dynrec_t *, dynrec_core_t*);
uint64_t dynrec_read_reg(const dynrec_core_t*, uint32_t);

void dynrec_destroy(dynrec_t *);
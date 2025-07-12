#pragma once
#include <pthread.h>
#include <stdint.h>

#include <algo/robin.h>
#include <algo/list.h>
typedef enum dynrec_cpu_type {
    dynrec_armv8a = 0,
    dynrec_aarch64 = 0,
    dynrec_armv4t = 1,
    dynrec_arm7tdmi = 1,
} dynrec_cpu_type_e;

typedef struct dynrec_frontend {
    size_t (*get_sizeof_gprs)();
    dynrec_cpu_type_e (*get_type)(const struct dynrec_frontend*);
} dynrec_frontend_t;


typedef struct dynrec {
    dynrec_frontend_t *frontend_ctx;
    list_t *cores_list;

    pthread_mutex_t monitor_mutex;

    int32_t int_enabled;
    uint8_t *memory;

    robin_map_t *flow_cfg_blocks;
} dynrec_t;
typedef struct dynrec_core {
    uint8_t * gprs_array;

    dynrec_t *jit;
} dynrec_core_t;

dynrec_t *dynrec_create(dynrec_cpu_type_e type);

dynrec_core_t * dynrec_enablecore(dynrec_t*);
void dynrec_disablecore(const dynrec_t *, dynrec_core_t*);

void dynrec_destroy(dynrec_t *);
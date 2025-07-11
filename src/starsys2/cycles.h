#pragma once
#include <sony.h>

typedef enum device_type {
    device_type_ee,
    device_type_iop,
    device_type_dmac
} device_type_e;

typedef enum affinity_default_type {
    affinity_default_mips_first
} affinity_default_type_e;

typedef void (*device_step_func_t)(void *, size_t*);

typedef struct device_step {
    void * target_dev;
    device_step_func_t stepdev;

    size_t *ticks;
} device_step_t;

typedef struct cycles {
    size_t ee_cycles;
    size_t iop_cycles;
    size_t dmac_cycles;

    sony_t * sony;

    device_type_e last_executed;
    size_t count;

    device_step_t **devices;
    size_t devices_count;

    device_type_e *affinity;
    bool affinity_isrng;
} cycles_t;


cycles_t * cycles_create(sony_t *);
void cycles_set_affinity_default(cycles_t *, affinity_default_type_e);
void cycles_set_affinity(cycles_t *, const char*);
const char * cycles_get_affinity_str(const cycles_t *);
void cycles_destroy(cycles_t *);
void cycles_step_devs(cycles_t *);
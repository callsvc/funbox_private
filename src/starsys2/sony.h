#pragma once

#include <ee/ee.h>
#include <ee/dmac.h>
#include <iop/mips.h>


typedef struct sony {
    procinfo_t *procinfo;
    ee_t *mips;
    mips_t *iop;
    dmac_t *dmac;

    bridge_t *bridge;
    uint8_t *ee_memory;
    vector_t *unkpaddr;

    const char *firmpath;
} sony_t;

sony_t * sony_create();
void sony_reset_cpus(const sony_t*);
void sony_destroy(sony_t*);
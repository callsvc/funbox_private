#pragma once
#include <stdint.h>

#include <algo/vector.h>
#include <bridge.h>
typedef struct ee ee_t;

typedef enum dmac_channel_type {
    dmac_channel_vif0, // vu0
    dmac_channel_vif1, // can act as PATH2 for GIF
    dmac_channel_gif, // path3
    dmac_channel_ipu_from, // video
    dmac_channel_ipu_to,
    dmac_channel_sif0, // from IOP SIF0 (IOP → EE)
    dmac_channel_sif1, // to IOP SIF1 (EE → IOP)
    dmac_channel_sif2, // bidirectional, used for PSX mode and debugging
    dmac_channel_spr_from, // scratchpad
    dmac_channel_spr_to,
} dmac_channel_type_e;

typedef struct dmac_channel {
    dmac_channel_type_e type;

    uint32_t chcr; // Channel control (R/W)
    uint32_t madr; // Channel address (R/W)
    uint32_t tadr; // Channel tag address (R/W)
    uint32_t qwc; // Quadword count (R/W)
    uint32_t asr0; // Channel saved tag address (R/W)
    uint32_t asr1;

    // SADR is only used by SPR_FROM and SPR_TO.
    uint32_t sadr; // Channel scratchpad address (R/W)
} dmac_channel_t;

typedef struct dmac {
    vector_t *channels_array;
    uint32_t d_ctrl; // DMAC control (R/W)

    bridge_t *bridge;
    ee_t *ee_core; // to raise an exception when the DMAC transfer is completed
} dmac_t;

dmac_t *dmac_create(ee_t *);
void dmac_run(const dmac_t *);

uint32_t dmac_read32(const dmac_t *, uint32_t);
void dmac_write32(dmac_t *, uint32_t addr,uint32_t);
void dmac_destroy(dmac_t *);

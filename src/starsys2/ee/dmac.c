#include <ee/dmac.h>

#include <core/types.h>

#include <sony.h>
#include <string.h>
dmac_t *dmac_create(ee_t *ee) {
    dmac_t * dmac = funbox_malloc(sizeof(dmac_t));
    dmac->ee_core = ee;
    dmac->bridge = ee->bridge;

    const int32_t count = dmac_channel_spr_to - dmac_channel_vif0 + 1;
    dmac->channels_array = vector_create(count, sizeof(dmac_channel_t));
    vector_setsize(dmac->channels_array, count);
    dmac_channel_type_e _chantype = dmac_channel_vif0;
    for (size_t i = 0; i < vector_size(dmac->channels_array); i++) {
        dmac_channel_t *channel = vector_get(dmac->channels_array, i);
        channel->type = _chantype++;
    }

    return dmac;
}

void dmac_run(const dmac_t *dmac, size_t *cycles) {
    if ((dmac->d_ctrl & 1) == 0)
        return;
    for (; *cycles; (*cycles)--) {}

    oskill("DMAC is required");
}
const int32_t dmac_ctrl_addr = 0x1000E000;
uint32_t dmac_read32(const dmac_t *dmac, const uint32_t addr) {
    if (addr == dmac_ctrl_addr)
        return dmac->d_ctrl;
    return 0;
}

bool contains_unkpaddr(const void * data, const size_t index, const void *user) {
    (void)index;
    uint32_t addr, target;
    memcpy(&addr, data, 4);
    memcpy(&target, user, 4);
    return addr == target;
}


void dmac_write32(dmac_t *dmac, const uint32_t addr, const uint32_t value) {
    if (addr == dmac_ctrl_addr)
        dmac->d_ctrl = value;

    bool known;
    const sony_t *sony = dmac->bridge->pstwo;
    vector_foreach(sony->unkpaddr, contains_unkpaddr, &value, known);
    if (!known)
        vector_emplace(sony->unkpaddr, &value);
    fprintf(stderr, "unknown DMAC addr: %#x\n", addr);
}

void dmac_destroy(dmac_t *dmac) {
    vector_destroy(dmac->channels_array);
    funbox_free(dmac);
}

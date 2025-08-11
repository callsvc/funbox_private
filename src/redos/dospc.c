#include <assert.h>
#include <stdlib.h>
#include <string.h>

#include <dospc.h>
dospc_t* dospc_create() {
    dospc_t* pc = fb_malloc(sizeof(dospc_t));
    pc->barrier = fb_rand(); // our cannary value
    pc->cannary = pc->barrier;

    pc->cpu = pc8086_create();
    pc->disk_slot = disk_create();

    return pc;
}

void dospc_reset(dospc_t *pc) {
    pc->cpu->pc = pc;

    auto p = (uint32_t*)pc->pc_memory;
    for (const uint32_t *end = &pc->barrier - 4; p < end; p++)
        *p = 0x0B16B005;

    disk_reset(pc->disk_slot, "FD14BOOT.img");
    pc8086_reset(pc->cpu);

    disk_read_sector(pc->disk_slot, 0, &pc->pc_memory[0x7C00]);
}

void dospc_continue(const dospc_t *pc) {
    while (!pc->cpu->int_)
        pc8086_run(pc->cpu);
}

void dospc_destroy(dospc_t *pc) {
    assert(pc->barrier == pc->cannary);
    pc8086_destroy(pc->cpu);
    disk_destroy(pc->disk_slot);
    fb_free(pc);
}

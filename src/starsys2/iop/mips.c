#include <stdio.h>
#include <types.h>
#include <iop/mips.h>
mips_t * mips_create(bridge_t * board) {
    mips_t * mips = funbox_malloc(sizeof(mips_t));
    mips->bridge = board;
    return mips;
}
void mips_reset(mips_t *mips) {
    mips->pc = 0xBFC00000;
}

uint32_t mips_solve_paddr(const uint32_t addr) {
    // Kernel segment 1
    if (addr >= 0xA0000000 && addr < 0xBFFFFFFF)
        return addr & 0x1FFFFFFF;
    return addr;
}

uint32_t mips_read32(const mips_t * mips, const uint32_t addr) {
    return bridge_read(mips->bridge, mips_solve_paddr(addr));
}

void mips_run(mips_t *mips, size_t *cycles) {
    for (; *cycles; (*cycles)--) {
        const uint32_t val = mips_read32(mips, mips->pc);
        fprintf(stderr, "iop, (%x) read from (%x)\n", val, mips->pc);
        mips->pc += 4;
    }
}

void mips_destroy(mips_t *mips) {
    funbox_free(mips);
}

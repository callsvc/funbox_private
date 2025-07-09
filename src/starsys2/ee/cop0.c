#include <stdio.h>
#include <string.h>
#include <ee/cop0.h>
void cop0_reset(cop0_t *cop) {
    memset(cop->regs, 0, sizeof(cop->regs));
    cop->prid = 0x59;
}

uint32_t cop0_mfc0(const cop0_t *cp0, const uint32_t regid) {
    if (regid >= COP0_REGS_COUNT)
        __builtin_trap();
    if (regid == COP_REG_PRID)
        return cp0->prid;
    return cp0->regs[regid];
}

void cop0_mtc0(cop0_t *cp0, const uint32_t id, const uint32_t value) {
    if (id == COP_REG_CONFIG)
        cp0->config = value;
    else cp0->regs[id] = value;
}

const char * cop0_rname(const uint32_t id) {
    static char copfmt[0x20];
    if (id == COP_REG_PRID)
        return "PRId";

    sprintf(copfmt, "%u", id);
    return copfmt;
}

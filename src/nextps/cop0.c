#include <stdio.h>
#include <string.h>

#include <types.h>
#include <cop0.h>
void cop0_reset(cop0_t *cop0) {
    memset(cop0->regs, 0, sizeof(cop0->regs));
}
// https://psx-spx.consoledev.net/cpuspecifications/#cop0-register-summary
const char * cop0_get_name(const size_t index) {
    switch (index) {
        case  3: return "bpc";
        case  6: return "tar";
        case  7: return "dcic";
        case 12: return "sr, (status register)";
        case 13: return "cause";
        case 15: return "prid";
        default:
            if (index <= 31)
                return "garbage";
    }
    return nullptr;
}

const uint8_t cop0_regs_wr[] = {
    0, 0, 0, 1, 0, 1, 0, 1,
    0, 1, 0, 1, 1, 0, 0, 0
};
void cop0_mtc(cop0_t *cop0, const uint32_t index, const uint32_t value) {
    printf("writing %x to the %s register in cop0\n", value, cop0_get_name(index));

    if (index <= 15 && !cop0_regs_wr[index]) {
        if (value)
            quit("can't write to this register");
        fprintf(stderr, "register number %u is read-only (value: %u)\n", index, value);
    }
    switch (index) {
        case 3: case 5: case 6: case 7: case 9: case 11:
            printf("attempt to write into a debug register in cop0, %u, %x\n", index, value);
        default:
            cop0->regs[index] = value;

    }
}

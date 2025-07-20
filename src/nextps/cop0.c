#include <stdio.h>
#include <string.h>

#include <cop0.h>
void cop0_reset(cop0_t *cop0) {
    memset(cop0->regs, 0, sizeof(cop0->regs));
}
// https://psx-spx.consoledev.net/cpuspecifications/#cop0-register-summary
const char * cop0_get_regname(const size_t index) {
    if (index == 12)
        return "status register (sr)";
    return nullptr;
}

void cop0_mtc(cop0_t *cop0, const uint32_t index, const uint32_t value) {
    printf("writing %x to the %s register in cop0\n", value, cop0_get_regname(index));

    switch (index) {
        case 3: case 5: case 6: case 7: case 9: case 11:
            printf("attempt to write to a debug register inside cop0, %u, %x\n", index, value);
        default:
            cop0->regs[index] = value;

    }
}

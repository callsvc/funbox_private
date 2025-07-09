#include <arm/dynrec.h>

int main() {
    dynrec_t *dynrec = dynrec_create(dynrec_aarch64);

    dynrec_core_t *enbcpu = dynrec_enablecore(dynrec);

    dynrec_disablecore(dynrec, enbcpu);
    dynrec_destroy(dynrec);
}
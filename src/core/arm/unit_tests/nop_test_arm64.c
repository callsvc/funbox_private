#include <stdio.h>

#include <arm/dynrec.h>
#include <arm/jit.h>
#include <arm/frontend/aarch64/arm64_types.h>
uint8_t memory_nop[1024];

int main() {
    for (size_t i = 0; i < sizeof(memory_nop) / 4; i++) {
        *(uint32_t*)&memory_nop[i * 4] = NOP2_IR_OPCODE;
    }

    dynrec_t *dynrec = dynrec_create(dynrec_type_aarch64);

    dynrec_core_t *main_cpu = dynrec_enablecore(dynrec);

    jit_set_memory(main_cpu, memory_nop, sizeof memory_nop);
    jit_run(main_cpu);

    const uint64_t lastpc = dynrec_read_reg(main_cpu, ARM64_PC_REGID);
    fprintf(stderr, "last pc: %ld\n", lastpc);

    dynrec_disablecore(dynrec, main_cpu);
    dynrec_destroy(dynrec);
}

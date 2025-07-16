#include <stdio.h>

#include <arm/jit.h>
#include <arm/ir/types.h>
#include <arm/frontend/aarch64/arm64_types.h>

uint32_t get_next_instruction(const dynrec_core_t *dyn_cpu) {
    const uint64_t next_pc = dyn_cpu->jit->frontend_ctx->get_pc(dyn_cpu->thc);
    const uint32_t inst = jit_read32(dyn_cpu, next_pc);

    dyn_cpu->thc->pc_reg += 4;
    return inst;
}
void arm64_generate_irs(const dynrec_core_t *dyn_cpu, jit_cfg_block_t *cfg) {
    dyn_cpu->thc->pc_reg = cfg->start_pc;
    if (cfg->irs == nullptr)
        cfg->irs = list_create(sizeof(ir_t));
    arm64_instruction_t slot_storage[10];

    const size_t inst_count = (cfg->end_pc - cfg->start_pc) / 4;
    fprintf(stderr, "generating irs for block (%p) at pc address (%ld) with %ld instructions\n", cfg, dyn_cpu->thc->pc_reg, inst_count);

    do {
        const uint32_t inst = get_next_instruction(dyn_cpu);
        arm64_lookup_table(slot_storage, inst);

        slot_storage->irgen(cfg->irs);
    } while (dyn_cpu->thc->pc_reg != cfg->end_pc);
}

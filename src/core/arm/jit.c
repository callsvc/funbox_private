#include <arm/types.h>
#include <arm/jit.h>

#include <core/types.h>

uint64_t jit_getpc(dynrec_core_t *core) {
    return core->jit->frontend_ctx->get_pc(core->gprs_array);
}

uint64_t jit_read_gpr(dynrec_core_t *core, uint64_t index) {
    return core->jit->frontend_ctx->read_gpr(core->gprs_array, index);
}
uint64_t jit_write_gpr(dynrec_core_t *core, uint64_t index, uint64_t value) {
    return core->jit->frontend_ctx->write_gpr(core->gprs_array, index, value);
}
uint64_t jit_write_neon(dynrec_core_t *core, uint64_t index, arm64_neon_t value) {
    return core->jit->frontend_ctx->write_vector(core->gprs_array, index, value);
}

uint8_t jit_read8(const dynrec_core_t *core, const uint64_t index) {
    return core->jit->memory[index];
}
uint16_t jit_read16(const dynrec_core_t *core, const uint64_t index) {
    return jit_read8(core, index) << 8 | jit_read8(core, index + 1);
}
uint32_t jit_read32(const dynrec_core_t *core, const uint64_t index) {
    return jit_read16(core, index) << 16 | jit_read16(core, index + 2);
}
uint64_t jit_read64(const dynrec_core_t *core, const uint64_t index) {
    return jit_read32(core, index) << 32 | jit_read32(core, index + 4);
}

jit_cfg_block_t * jit_compile(dynrec_core_t *core, const uint64_t start_pc) {
    uint64_t compiled = 0;
    dynrec_t *jit = core->jit;

    jit_cfg_block_t *first = nullptr;
    do {
        jit_cfg_block_t *cfg = fb_malloc(sizeof(jit_cfg_block_t));

        cfg->start_pc = start_pc;

        jit->frontend_ctx->generate_irs(core->jit, cfg);
        compiled += vector_size(cfg->irs);

        jit->backend_ctx->generate_code(core->jit, cfg);

        if (first)
            list_push(first->nested_blocks, cfg);
        else first = cfg;
    } while (compiled < 1024);
    return first;
}


void jit_execute(const jit_cfg_block_t *start_block) {
    if (start_block->has_entrypoint)
        return;
    for (size_t i = 0; i < list_size(start_block->nested_blocks); i++) {
        const jit_cfg_block_t *next = list_get(start_block->nested_blocks, i);
        (void)next;
    }
}

void jit_run(dynrec_core_t *core) {
    const uint64_t pc = jit_getpc(core);
    const uint64_t start_pc = pc & ~1000;
    const dynrec_t *jit = core->jit;

    while (!jit->int_enabled) {
        jit_cfg_block_t *cfg = robin_map_get(jit->flow_cfg_blocks, (void*)start_pc);
        if (!cfg) {
            cfg = jit_compile(core, pc);
            robin_map_emplace(jit->flow_cfg_blocks, (void*)start_pc, cfg);
        }
        jit_execute(cfg);
    }
}


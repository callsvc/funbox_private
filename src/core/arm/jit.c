#include <arm/jit.h>
#include <core/types.h>

uint64_t jit_getpc(const dynrec_core_t *dyn_cpu) {
    return dyn_cpu->jit->frontend_ctx->get_pc(dyn_cpu->thc);
}
uint64_t jit_read_gpr(const dynrec_core_t *dyn_cpu, const uint64_t index) {
    return dyn_cpu->jit->frontend_ctx->read_gpr(dyn_cpu->thc, index);
}
uint8_t jit_read8(const dynrec_core_t *dyn_cpu, const uint64_t index) {
    return dyn_cpu->jit->memory[index];
}
uint16_t jit_read16(const dynrec_core_t *dyn_cpu, const uint64_t index) {
    return (uint8_t)(jit_read8(dyn_cpu, index) << 8) | jit_read8(dyn_cpu, index + 1);
}
uint32_t jit_read32(const dynrec_core_t *dyn_cpu, const uint64_t index) {
    return (uint32_t)jit_read16(dyn_cpu, index) << 16 | jit_read16(dyn_cpu, index + 2);
}
uint64_t jit_read64(const dynrec_core_t *dyn_cpu, const uint64_t index) {
    return (uint64_t)jit_read32(dyn_cpu, index) << 32 | jit_read32(dyn_cpu, index + 4);
}

jit_cfg_block_t * jit_compile(const dynrec_core_t *dyn_cpu, const uint64_t start_pc) {
    uint64_t compiled = 0;
    const dynrec_t *jit = dyn_cpu->jit;

    jit_cfg_block_t *first = nullptr;
    do {
        jit_cfg_block_t *cfg = fb_malloc(sizeof(jit_cfg_block_t));

        cfg->start_pc = start_pc;
        cfg->end_pc = start_pc + 0x200;

        jit->frontend_ctx->generate_irs(dyn_cpu, cfg);
        compiled += list_size(cfg->irs);

        // jit->backend_ctx->generate_code(core->jit, cfg);

        if (first) {
            if (!first->nested_blocks)
                first->nested_blocks = list_create(0);
            list_push(first->nested_blocks, cfg);
        } else {
            first = cfg;
        }
    } while (compiled < 1024);
    return first;
}


void jit_cleanup(const dynrec_core_t *dyn_cpu, jit_cfg_block_t *start_block) {
    dyn_cpu->jit->int_enabled = true;
    if (start_block->has_entrypoint)
        return;

    if (start_block->nested_blocks) {
        for (size_t i = 0; i < list_size(start_block->nested_blocks); i++)
            jit_cleanup(dyn_cpu, list_get(start_block->nested_blocks, i));
        list_destroy(start_block->nested_blocks);
    }

    if (start_block->irs)
        list_destroy(start_block->irs);

    fb_free(start_block);
}

void jit_run(const dynrec_core_t *dyn_cpu) {
    const uint64_t pc = jit_getpc(dyn_cpu);
    const uint64_t start_pc = pc & ~1000;
    const dynrec_t *jit = dyn_cpu->jit;

    while (!jit->int_enabled) {
        jit_cfg_block_t *cfg = robin_map_get(jit->flow_cfg_blocks, to_str64(start_pc, 10));
        if (!cfg) {
            cfg = jit_compile(dyn_cpu, pc);
            robin_map_emplace(jit->flow_cfg_blocks, (void*)to_str64(start_pc, 10), cfg);
        }
        jit_cleanup(dyn_cpu, cfg);
        robin_map_emplace(jit->flow_cfg_blocks, (void*)to_str64(start_pc, 10), nullptr);
    }
}

void jit_set_memory(const dynrec_core_t *dyn_cpu, uint8_t *memory, const size_t size) {
    if (!dyn_cpu->jit)
        return;
    if (dyn_cpu->jit->memory)
        fb_free(dyn_cpu->jit->memory);
    dyn_cpu->jit->memory = memory;
    dyn_cpu->jit->memory_size = size;
}


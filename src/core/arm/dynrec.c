#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>

#include <types.h>
#include <arm/dynrec.h>

#include <arm/frontend/aarch64/arm64_types.h>
dynrec_t * dynrec_create(const dynrec_cpu_type_e type) {
    dynrec_t *jit = fb_malloc(sizeof(dynrec_t));
    jit->cores_list = list_create(0);

    uint8_t types[] = {0, 2};
    jit->flow_cfg_blocks = robin_map_create(types);

    if (type == dynrec_type_aarch64)
        jit->frontend_ctx = (dynrec_frontend_t*)dynrec_frontend_arm64_create(jit);
    return jit;
}

dynrec_core_t * dynrec_enablecore(dynrec_t *jit) {
    cpu_set_t cpuset;
    sched_getaffinity(getpid(), sizeof(cpuset), &cpuset);
    if (list_size(jit->cores_list) == CPU_COUNT(&cpuset))
        return nullptr;

    if (!jit->frontend_ctx)
        return nullptr;

    dynrec_core_t * dyn_cpu = fb_malloc(sizeof(dynrec_core_t));
    dyn_cpu->thc = th_context_create(jit);
    dyn_cpu->jit = jit;
    list_push(jit->cores_list, dyn_cpu);

    return dyn_cpu;
}
void dynrec_disablecore(const dynrec_t * jit, dynrec_core_t *dyn_cpu) {
    for (size_t i = 0; i < list_size(jit->cores_list); i++)
        if (list_get(jit->cores_list, i) == dyn_cpu)
            list_drop(jit->cores_list, i);

    th_context_destroy(dyn_cpu->thc);
    fb_free(dyn_cpu);
}

uint64_t dynrec_read_reg(const dynrec_core_t *dyn_cpu, const uint32_t id) {
    if (dyn_cpu->jit)
        return 0;

    if (id == ARM64_PC_REGID)
        return dyn_cpu->jit->frontend_ctx->get_pc(dyn_cpu->thc);
    return dyn_cpu->jit->frontend_ctx->read_gpr(dyn_cpu->thc, id);
}

void dynrec_destroy(dynrec_t *jit) {
    for (size_t i = 0; i < list_size(jit->cores_list); i++) {
        dynrec_core_t *core = list_get(jit->cores_list, i);
        dynrec_disablecore(jit, core);
        list_drop(jit->cores_list, i);
    }

    list_destroy(jit->cores_list);
    robin_map_destroy(jit->flow_cfg_blocks);

    if (jit->frontend_ctx->get_type(jit->frontend_ctx) == dynrec_type_aarch64) {
        dynrec_frontend_t * front = jit->frontend_ctx;
        jit->frontend_ctx = nullptr;
        dynrec_frontend_arm64_destroy((dynrec_frontend_arm64_t*)front);
    }

    fb_free(jit);
}

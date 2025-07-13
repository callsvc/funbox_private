#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>

#include <types.h>
#include <arm/dynrec.h>


#include <arm/frontend/aarch64/arm64_frontend.h>

dynrec_t *dynrec_create(const dynrec_cpu_type_e type) {
    dynrec_t *jit = fb_malloc(sizeof(dynrec_t));
    jit->cores_list = list_create(0);

    uint8_t types[] = {1, 2};
    jit->flow_cfg_blocks = robin_map_create(types);

    if (type == dynrec_aarch64)
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

    dynrec_core_t * earlycore = fb_malloc(sizeof(dynrec_core_t));
    earlycore->gprs_array = fb_malloc(jit->frontend_ctx->get_sizeof_gprs());
    earlycore->jit = jit;

    list_push(jit->cores_list, earlycore);

    return earlycore;
}
void dynrec_disablecore(const dynrec_t * jit, dynrec_core_t *core) {
    for (size_t i = 0; i < list_size(jit->cores_list); i++)
        if (list_get(jit->cores_list, i) == core)
            list_drop(jit->cores_list, i);

    fb_free(core->gprs_array);
    fb_free(core);
}
void dynrec_destroy(dynrec_t *jit) {
    for (size_t i = 0; i < list_size(jit->cores_list); i++) {
        dynrec_core_t *core = list_get(jit->cores_list, i);
        fb_free(core->gprs_array);
        list_drop(jit->cores_list, i);
        fb_free(core);
    }

    list_destroy(jit->cores_list);
    robin_map_destroy(jit->flow_cfg_blocks);

    if (jit->frontend_ctx->get_type(jit->frontend_ctx) == dynrec_aarch64) {
        dynrec_frontend_t * front = jit->frontend_ctx;
        jit->frontend_ctx = nullptr;
        dynrec_frontend_arm64_destroy((dynrec_frontend_arm64_t*)front);
    }

    fb_free(jit);
}
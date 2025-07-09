#define _GNU_SOURCE
#include <unistd.h>
#include <sched.h>
#include <stdlib.h>

#include <types.h>
#include <arm/dynrec.h>


#include <arm/frontend/aarch64/arm64_frontend.h>

dynrec_t *dynrec_create(const dynrec_cpu_type_e type) {
    dynrec_t *jit = funbox_malloc(sizeof(dynrec_t));
    jit->cores_list = list_create(0);

    if (type == dynrec_aarch64)
        jit->frontend_ctx = (dynrec_frontend_t*)dynrec_frontend_arm64_create(jit);
    return jit;
}

dynrec_core_t * dynrec_enablecore(dynrec_t *jit) {
    cpu_set_t cpuset;
    sched_getaffinity(getpid(), sizeof(cpuset), &cpuset);
    if (list_size(jit->cores_list) == CPU_COUNT(&cpuset))
        return NULL;

    if (!jit->frontend_ctx)
        return NULL;

    dynrec_core_t * earlycore = funbox_malloc(sizeof(dynrec_core_t));
    earlycore->gprs_array = funbox_malloc(jit->frontend_ctx->get_sizeof_gprs());
    earlycore->jit = jit;

    list_push(jit->cores_list, earlycore);

    return earlycore;
}
void dynrec_disablecore(const dynrec_t * jit, dynrec_core_t *core) {
    for (size_t i = 0; i < list_size(jit->cores_list); i++)
        if (list_get(jit->cores_list, i) == core)
            list_drop(jit->cores_list, i);

    free(core->gprs_array);
    funbox_free(core);
}
void dynrec_destroy(dynrec_t *jit) {
    for (size_t i = 0; i < list_size(jit->cores_list); i++) {
        dynrec_core_t *core = list_get(jit->cores_list, i);
        free(core->gprs_array);
        list_drop(jit->cores_list, i);
        free(core);
    }

    list_destroy(jit->cores_list);

    if (jit->frontend_ctx->get_type(jit->frontend_ctx) == dynrec_aarch64) {
        dynrec_frontend_t * front = jit->frontend_ctx;
        jit->frontend_ctx = NULL;
        dynrec_frontend_arm64_destroy((dynrec_frontend_arm64_t*)front);
    }

    funbox_free(jit);
}
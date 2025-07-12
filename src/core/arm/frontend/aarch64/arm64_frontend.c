#include <assert.h>
#include <stdint.h>

#include <types.h>
#include <arm/types.h>
#include <arm/frontend/aarch64/arm64_frontend.h>

size_t arm64_get_sizeof_gprs() {
    return 31 * sizeof(uint64_t) + 32 * sizeof(arm64_neon_t);
}
dynrec_cpu_type_e arm64_get_type(const struct dynrec_frontend *) {
    return dynrec_aarch64;
}

dynrec_frontend_arm64_t * dynrec_frontend_arm64_create(dynrec_t *jit) {
    dynrec_frontend_arm64_t * arm64front = funbox_malloc(sizeof(dynrec_frontend_arm64_t));
    arm64front->parent = jit;

    arm64front->funcslist.get_sizeof_gprs = arm64_get_sizeof_gprs;
    arm64front->funcslist.get_type = arm64_get_type;

    return arm64front;
}
void dynrec_frontend_arm64_destroy(dynrec_frontend_arm64_t *arm64front) {
    assert(arm64front->parent->frontend_ctx != (dynrec_frontend_t*)arm64front);
    funbox_free(arm64front);
}
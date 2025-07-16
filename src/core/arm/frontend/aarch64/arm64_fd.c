#include <assert.h>
#include <stdint.h>

#include <types.h>
#include <arm/frontend/aarch64/arm64_types.h>

uint64_t arm64_read_gpr(const th_context_t *thc, const uint64_t index) {
    return (uint64_t)vector_get(thc->regs, index);
}
uint64_t arm64_get_pc(const th_context_t *thc) {
    return thc->pc_reg;
}
size_t arm64_get_gprs_count() {
    return 31;
}
dynrec_cpu_type_e arm64_get_type(const struct dynrec_frontend *) {
    return dynrec_type_aarch64;
}

dynrec_frontend_arm64_t * dynrec_frontend_arm64_create(dynrec_t *jit) {
    dynrec_frontend_arm64_t * frontarm = fb_malloc(sizeof(dynrec_frontend_arm64_t));
    frontarm->jit_gen = jit;

    frontarm->funcslist.get_gprs_count = arm64_get_gprs_count;
    frontarm->funcslist.get_type = arm64_get_type;
    frontarm->funcslist.read_gpr = arm64_read_gpr;
    frontarm->funcslist.get_pc = arm64_get_pc;

    frontarm->funcslist.generate_irs = arm64_generate_irs;

    return frontarm;
}
void dynrec_frontend_arm64_destroy(dynrec_frontend_arm64_t *frontarm) {
    assert(frontarm->jit_gen->frontend_ctx != (dynrec_frontend_t*)frontarm);
    fb_free(frontarm);
}
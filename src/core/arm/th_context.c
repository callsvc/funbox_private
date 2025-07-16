#include <types.h>
#include <arm/dynrec.h>
#include <arm/th_context.h>


th_context_t *th_context_create(const dynrec_t * jit) {
    th_context_t *thc = fb_malloc(sizeof(th_context_t));

    thc->regs = vector_create(0, sizeof(uint64_t));
    vector_setsize(thc->regs, jit->frontend_ctx->get_gprs_count());
    return thc;
}
void th_context_destroy(th_context_t *thc) {
    vector_destroy(thc->regs);
    fb_free(thc);
}
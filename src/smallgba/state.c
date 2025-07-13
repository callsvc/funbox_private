#include <state.h>

#include <types.h>

state_t * state_create() {
    state_t * state = fb_malloc(sizeof(state_t));
    state->cart = cartridge_create();
    return state;
}
void state_destroy(state_t *state) {
    cartridge_destroy(state->cart);
    fb_free(state);
}
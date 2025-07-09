#include <state.h>

// https://gbadev.net/gbadoc/memory.html
int main() {
    state_t * state = state_create();

    cartridge_load_rom(state->cart, "tetris.gba");

    state_destroy(state);
}

#pragma once

#include <cartridge.h>
typedef struct state {
    cartridge_t *cart;
} state_t;

state_t * state_create();
void state_destroy(state_t *);
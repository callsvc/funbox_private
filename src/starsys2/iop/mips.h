#pragma once
#include <stdint.h>
#include <bridge.h>

// https://www.psdevwiki.com/ps2/IOP/Deckard
typedef struct mips {
    bridge_t *bridge;
    uint32_t pc;
} mips_t;

mips_t * mips_create(bridge_t *);
void mips_reset(mips_t*);
void mips_run(mips_t*, size_t *cycles);
void mips_destroy(mips_t*);

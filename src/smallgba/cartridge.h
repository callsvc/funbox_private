#pragma once
#include <stdint.h>
#include <algo/vector.h>

typedef struct rom_header {
    uint32_t rom_entry;
    uint8_t nintendo_logo[156];
    uint8_t game_title[12];
} rom_header_t;

typedef struct cartridge {
    vector_t *content;
} cartridge_t;

cartridge_t *cartridge_create();
void cartridge_load_rom(cartridge_t*, const char *);
void cartridge_destroy(cartridge_t *);
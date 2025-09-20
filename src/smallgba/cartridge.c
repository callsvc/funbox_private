#include <cartridge.h>
#include <string.h>

#include <types.h>
#include <fs/file.h>

cartridge_t *cartridge_create() {
    cartridge_t * cart = fb_malloc(sizeof(cartridge_t));
    return cart;
}

void cartridge_load_rom(cartridge_t *cart, const char *path) {
    file_t *rom = file_open(path, "r", false);
    if (!rom)
        quit("can't open rom");
    cart->content = fs_filebytes((fsfile_t*)rom);
    file_close(rom);

    uint8_t *start_rom = vector_begin(cart->content);
    const rom_header_t *header = (rom_header_t*)start_rom;
    char rom_title[13] = {};
    memcpy(rom_title, header->game_title, 12);

    printf("ROM title: %s\n", rom_title);

    int32_t sum_val = 0;
    for (uint16_t addr = 0xA0; addr <= 0xBC; ++addr)
        sum_val += start_rom[addr];
    if (start_rom[0xBD] != 0x100 - (sum_val % 0x100 & 0xFF))
        fprintf(stderr, "this ROM is corrupted!\n");

}
void cartridge_destroy(cartridge_t *cart) {
    if (cart->content)
        vector_destroy(cart->content);
    fb_free(cart);
}
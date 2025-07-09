#include <fs/file.h>
#include <sony.h>
#include <bridge.h>


bridge_t * bridge_create(sony_t *sony) {
    bridge_t *board = funbox_malloc(sizeof(bridge_t));

    board->pstwo = sony;
    sony->ee_memory = funbox_malloc(sizeof(uint8_t) * 32 * 1024 * 1024);

    if (!sony->firmpath)
        oskill("a firmware is needed to continue");

    file_t *file = file_open(sony->firmpath, "r");
    if (!file)
        oskill("can't open the firmware file: %s", file_errorpath(sony->firmpath));

    file_read(file, sony->ee_memory, 0x400000, 0);
    file_close(file);

    return board;
}
void bridge_destroy(bridge_t * board) {
    funbox_free(board->pstwo->ee_memory);

    board->pstwo->ee_memory = NULL;

    funbox_free(board);
}

uint8_t * firmware_program(uint8_t * memory, const uint32_t addr) {
    const uint32_t bios_mask = 0x3FFFFF;
    return &memory[addr & bios_mask];
}
bool isdmac(const uint32_t addr) {
    if (addr >> 16 == 0x1000)
        if ((addr & 0xFFFF) <= 0xF590)
            return true;
    return false;
}

bool isfirmwarero(const uint32_t addr) {
    const uint32_t firm_start = 0x1FC00000; // KSEG1
    const uint32_t firm_end = 0x20000000;

    return addr >= firm_start && addr < firm_end;
}

uint32_t bridge_read(const bridge_t *board, const uint32_t addr) {
    uint8_t *place = NULL;
    if (isfirmwarero(addr))
        place = firmware_program(board->pstwo->ee_memory, addr);

    if (!place && isdmac(addr))
        dmac_read32(board->pstwo->dmac, addr);

    return place ? *(uint32_t*)place : 0;
}

void bridge_write(const bridge_t *board, const uint32_t addr, const uint32_t value) {
    if (isdmac(addr))
        dmac_write32(board->pstwo->dmac, addr, value);
    else
        board->pstwo->ee_memory[addr] = value;
}
#include <types.h>
#include <fs/file.h>


int main() {
    file_t *romfile = file_open("Disk.z64", "r");

    uint8_t buffer[512];
    fs_read((fsfile_t*)romfile, &buffer, sizeof(buffer), 0);
    fprintf(stderr, "N64 title to be loaded: %s\n", (char*)&buffer[0x20]);

    const uint32_t bentry = swap_b32((uint32_t*)&buffer[0x8]);
    fprintf(stderr, "Rom entry point: %#x\n", bentry);

    file_close(romfile);
}

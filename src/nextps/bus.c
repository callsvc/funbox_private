#include <string.h>
#include <mbedtls/sha1.h>

#include <ps.h>
#include <types.h>
#include <fs/file.h>
bool cmpsha(const uint8_t *bytes, const char *sha) {
    mbedtls_sha1_context sha1;
    mbedtls_sha1_init(&sha1);

    mbedtls_sha1_starts(&sha1);
    mbedtls_sha1_update(&sha1, bytes, 512 * 1024);

    uint8_t result[20];
    mbedtls_sha1_finish(&sha1, result);

    uint8_t against[20];
    strtobytes(sha, against, 20);

    mbedtls_sha1_free(&sha1);

    return memcmp(result, against, sizeof(result)) == 0;
}

bus_t *bus_create() {
    bus_t *bus = fb_malloc(sizeof(bus_t));
    bus->mips_memory = fb_malloc(4 * 1024 * 1024);

    file_t *bios_scph = file_open("scph1001.bin", "r");
    fs_read((fsfile_t*)bios_scph, bus->mips_memory, fs_getsize((fsfile_t*)bios_scph), 0);

    if (!cmpsha(bus->mips_memory, "10155D8D6E6E832D6EA66DB9BC098321FB5E8EBF"))
        oskill("bios file is corrupted");

    file_close(bios_scph);

    return bus;
}
void bus_destroy(bus_t *bus) {
    fb_free(bus->mips_memory);
    fb_free(bus);
}

#include <assert.h>
#include <ps.h>
#include <types.h>
#include <fs/file.h>

bus_t *bus_create() {
    bus_t *bus = fb_malloc(sizeof(bus_t));
    bus->mips_memory = fb_malloc(4 * 1024 * 1024);

    file_t *bios_scph = file_open("scph1001.bin", "r");
    fs_read((fsfile_t*)bios_scph, bus->mips_memory, fs_getsize((fsfile_t*)bios_scph), 0);

    if (!cmpsha(bus->mips_memory, 512 * 1024, "10155D8D6E6E832D6EA66DB9BC098321FB5E8EBF"))
        oskill("bios file is corrupted");
    file_close(bios_scph);
    return bus;
}

#define PSX_RAM_SIZE 0x200000

constexpr uint32_t bios_mask = 0x80000 - 1;
constexpr uint32_t ram_mask = PSX_RAM_SIZE - 1;
uint32_t bus_read(const bus_t *bus, const uint32_t address) {
    if (address >= 0xBFC00000 && address < 0xBFC80000)
        return *(uint32_t*)&bus->mips_memory[address & bios_mask];

    if (address >= 0xA0000000 && address < 0xA0200000) {
        return *(uint32_t*)&bus->ram_memory[address & ram_mask];
    }

    return 0;
}

// https://psx-spx.consoledev.net/memorycontrol/
void bus_write(bus_t *bus, const uint32_t address, const uint32_t value) {
    if (address >= 0xA0000000 && address < 0xA0200000) {
        *(uint32_t*)&bus->ram_memory[address & ram_mask] = value;
    }

    switch (address) {
        case 0x1F801000: // Expansion 1 Base Address
            assert(value == 0x1F000000);
            bus->expaddrs[0] = value; break;
        case 0x1F801010: // BIOS ROM Delay/Size
            assert(value == 0x0013243F); break;
        case 0x1F801060:
            bus->ram_size = value;
            if (!bus->ram_memory)
                bus->ram_memory = fb_malloc(PSX_RAM_SIZE);
            for (size_t i = 0; i < PSX_RAM_SIZE; i += 4) {
                *(uint32_t*)(bus->ram_memory + i) = 0xDEADC0DE;
            }
            break;
        case 0xFFFE0130:
            printf("cache control not implemented\n"); break;

        default:
    }
}

void bus_destroy(bus_t *bus) {
    fb_free(bus->mips_memory);
    fb_free(bus->ram_memory);
    fb_free(bus);
}

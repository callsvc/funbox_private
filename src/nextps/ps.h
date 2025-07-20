#pragma once
#include <stdint.h>


typedef struct bus {
    uint8_t *mips_memory;
    uint8_t *ram_memory;

    uint32_t expaddrs[1];
    uint32_t ram_size;
} bus_t;


bus_t *bus_create();
uint32_t bus_read(const bus_t *bus, uint32_t address);
void bus_write(bus_t *bus, uint32_t address, uint32_t value);

void bus_destroy(bus_t *);


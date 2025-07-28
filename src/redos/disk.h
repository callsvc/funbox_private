#pragma once

#include <types.h>
#include <fs/file.h>
typedef struct disk {
    file_t *file;
} disk_t;


disk_t * disk_create();
void disk_reset(disk_t *, const char*);
void disk_read_sector(const disk_t*, uint16_t, uint8_t *);
void disk_destroy(disk_t*);
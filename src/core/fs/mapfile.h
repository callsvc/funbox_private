#pragma once
#include <fs/types.h>


typedef struct mapfile {
    fsfile_t vfile;
    int32_t fd;
    uint8_t *buffer;
    size_t size;
} mapfile_t;

mapfile_t * mapfile_open(const fsfile_t *);
mapfile_t * mapfile_open_2(const char *path, uint8_t *, size_t);
void mapfile_close(mapfile_t *);
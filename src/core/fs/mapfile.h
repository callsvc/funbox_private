#pragma once
#include <fs/types.h>


typedef struct mapfile {
    fsfile_t vfile;
    int32_t fd;
    uint8_t *buffer;
} mapfile_t;

mapfile_t * mapfile_open(const fsfile_t *);
void mapfile_close(mapfile_t *);
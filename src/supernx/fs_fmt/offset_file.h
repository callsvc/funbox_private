#pragma once
#include <fs/types.h>

typedef struct offset_file {
    fsfile_t vfile;

    fsfile_t *file;
    char *buffer;
    uint64_t start;
    size_t size;
} offset_file_t;

offset_file_t * offset_file_open(fsfile_t *base, const char *name, size_t size, uint64_t offset, bool);
void offset_file_close(const fsfile_t *, offset_file_t *);

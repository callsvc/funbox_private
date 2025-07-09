#pragma once
#include <stdint.h>

#include <fs/file.h>
typedef struct pkg_header {
    uint32_t pkg_magic;
    uint32_t pkg_type;
    uint8_t _pad[0x8];
    uint32_t pkg_file_count;

} __attribute__((aligned(0x1000))) pkg_header_t;

typedef struct pkg_loader {
    pkg_header_t file_header;
} pkg_loader_t;

pkg_loader_t * pkg_create(fsfile_t *file);
void pkg_destroy(pkg_loader_t*);
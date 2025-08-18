#pragma once
#include <stdint.h>

#include <fs/file.h>
#pragma pack(push, 1)
typedef struct pkg_header {
    uint32_t pkg_magic;
    uint32_t pkg_type;
    uint32_t pad0;
    uint32_t pkg_file_count;
    uint32_t pkg_entry_count;
    uint16_t pkg_sc_entry_count;
    uint16_t pkg_entry_count_2;
    uint32_t pkg_table_offset;

} pkg_header_t;
#pragma pack(pop)

typedef struct pkg_loader {
    file_t *pkg_file;
} pkg_loader_t;

pkg_loader_t * pkg_create(fsfile_t *file);
void pkg_destroy(pkg_loader_t*);
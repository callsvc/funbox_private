#pragma once
#include <fs/types.h>

#include <algo/list.h>

typedef struct romfs {
    fsdir_t vdir;

    fsfile_t *basefile;
    vector_t *files;
    list_t *override_files;

    uint8_t *metafiles, *metadirs;

    uint64_t data_offset;
    char pathbuild[0x1000];
} romfs_t;

romfs_t * romfs_create_2();
romfs_t *romfs_create(fsfile_t *);
void romfs_addfile(const romfs_t *rfs, fsfile_t *file);
size_t romfs_override(const romfs_t *, fsfile_t *, const char*);
void romfs_destroy(romfs_t *);
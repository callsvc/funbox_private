#pragma once
#include <fs/types.h>

// https://www.psdevwiki.com/ps4/PFS
typedef struct pfs_superblock {
    uint64_t version;
    uint32_t format;
    uint32_t id;
} pfs_superblock_t;

typedef struct pfs {
    fsfile_t *file;
} pfs_t;

pfs_t * pfs_create(fsfile_t *);
void pfs_destroy(pfs_t *);
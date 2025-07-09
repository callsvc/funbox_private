#pragma once

#include <stdbool.h>
#include <fs/types.h>

typedef struct pfs {
    fsdir_t vdir;

    fsfile_t *basefio;
    vector_t *files; // Contains all files resident in this pfs
    vector_t *files_paths;
} pfs_t;

typedef struct pfs_file {
    char filename[50];
    uint64_t offset;
    uint64_t size;
} pfs_file_t;

bool pfs_is_pfs(fsfile_t *);
pfs_t * pfs_create(fsfile_t *);
void pfs_print_files(const pfs_t *);
void pfs_destroy(pfs_t *);
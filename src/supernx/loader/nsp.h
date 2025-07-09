#pragma once

#include <stdbool.h>
#include <fs/types.h>
#include <fs_fmt/submission_package.h>
#include <loader/types.h>
typedef struct nsp {
    loader_base_t vloader;
    submission_package_t *next_loader;
} nsp_t;

nsp_t *nsp_create(fsfile_t *file, keys_db_t *);
bool nsp_is_nsp(fsfile_t *file);
void nsp_destroy(nsp_t *nsp);
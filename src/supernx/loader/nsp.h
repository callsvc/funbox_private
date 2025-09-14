#pragma once

#include <fs/types.h>
#include <fs_fmt/submission_package.h>
#include <loader/types.h>
typedef struct nsp {
    loader_base_t vloader;
    submission_package_t *nsp_main;
} nsp_t;

nsp_t *nsp_create(fsfile_t *, keys_db_t *);

bool nsp_is_nsp(fsfile_t *);

void nsp_destroy(nsp_t *nsp);
#pragma once
#include <fs_fmt/pfs.h>
#include <horizon/keys_db.h>


typedef struct submission_package {
    keys_db_t *keys;
    pfs_t *main_pfs;

    list_t *nca_list;
} submission_package_t;


submission_package_t * submission_package_create(fsfile_t *file, keys_db_t *);
void submission_package_destroy(submission_package_t *);
#pragma once
#include <fs_fmt/pfs.h>
#include <horizon/keys_db.h>

#include <fs_fmt/content_archive.h>

typedef struct submission_package {
    keys_db_t *keys;
    pfs_t *main_pfs;

    list_t *nca_list;
} submission_package_t;


submission_package_t * submission_package_create(fsfile_t *file, keys_db_t *);

content_archive_t * submission_package_nca_bytype(const submission_package_t*, content_type_e);
pfs_t * submission_package_pfs_byfile(const submission_package_t *, const char*);

void submission_package_destroy(submission_package_t *);
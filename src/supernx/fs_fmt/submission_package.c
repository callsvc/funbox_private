#include <string.h>
#include <types.h>
#include <horizon/tik.h>

#include <fs_fmt/submission_package.h>
#include <fs_fmt/content_archive.h>

void load_all_tiks(const submission_package_t *subp, const vector_t *tiks) {
    for (size_t i = 0; i < vector_size(tiks); i++) {
        fsfile_t * file = fs_open_file((fsdir_t*)subp->main_pfs, vector_get(tiks, i), "r");

        const tik_t *ticket = tik_create(file);
        fs_close_file((fsdir_t*)subp->main_pfs, file);

        keys_db_add_ticket(subp->keys, ticket);
    }
}

submission_package_t * submission_package_create(fsfile_t *file, keys_db_t *keys) {
    submission_package_t * subp = funbox_malloc(sizeof(submission_package_t));
    subp->keys = keys;
    subp->main_pfs = pfs_create(file);
    subp->nca_list = list_create(0);

    if (subp->main_pfs)
        pfs_print_files(subp->main_pfs);

    vector_t *tiks = fs_grep((fsdir_t*)subp->main_pfs, "*.tik");
    load_all_tiks(subp, tiks);

    vector_t * files = fs_list_all_files((fsdir_t*)subp->main_pfs);
    for (size_t i = 0; i < vector_size(files); i++) {
        const char * ncapath = vector_get(files, i);
        if (strchr(ncapath, '.') != NULL)
            if (strcmp(ncapath + strlen(ncapath) - 4, ".nca") != 0)
                continue;

        list_push(subp->nca_list, content_archive_create(keys, (fsdir_t*)subp->main_pfs, ncapath));
    }
    vector_destroy(files);
    vector_destroy(tiks);

    return subp;
}
void submission_package_destroy(submission_package_t *subp) {
    for (size_t i = 0; i < list_size(subp->nca_list); i++) {
        content_archive_destroy(list_get(subp->nca_list, i));
    }
    list_destroy(subp->nca_list);
    pfs_destroy(subp->main_pfs);
    funbox_free(subp);
}
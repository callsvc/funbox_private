#include <string.h>
#include <types.h>
#include <horizon/tik.h>
#include <fs_fmt/submission_package.h>

void load_all_tiks(const submission_package_t *subp, const vector_t *tiks) {
    for (size_t i = 0; i < vector_size(tiks); i++) {
        fsfile_t * file = fs_open_file((fsdir_t*)subp->main_pfs, vector_get(tiks, i), "r");

        const tik_t *ticket = tik_create(file);
        fs_close_file((fsdir_t*)subp->main_pfs, file);

        keys_db_add_ticket(subp->keys, ticket);
    }
}

submission_package_t * submission_package_create(fsfile_t *file, keys_db_t *keys) {
    submission_package_t * subp = fb_malloc(sizeof(submission_package_t));
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
        if (strchr(ncapath, '.') != nullptr)
            if (strcmp(ncapath + strlen(ncapath) - 4, ".nca") != 0)
                continue;

        list_push(subp->nca_list, content_archive_create(keys, (fsdir_t*)subp->main_pfs, ncapath));
    }
    vector_destroy(files);
    vector_destroy(tiks);

    return subp;
}

content_archive_t * submission_package_nca_bytype(const submission_package_t *subp, const content_type_e type) {
    for (size_t i = 0; i < list_size(subp->nca_list); i++) {
        content_archive_t * nca = list_get(subp->nca_list, i);
        if (nca->type == type)
            return nca;
    }
    return nullptr;
}

pfs_t * submission_package_pfs_byfile(const submission_package_t * subp, const char * filename) {
    for (size_t i = 0; i < list_size(subp->nca_list); i++) {
        const content_archive_t * nca = list_get(subp->nca_list, i);
        for (size_t pfs_index = 0; content_archive_get_pfs(nca, pfs_index); pfs_index++) {
            pfs_t * pfs_file = content_archive_get_pfs(nca, pfs_index);

            vector_t *files = fs_list_all_files((fsdir_t*)pfs_file);
            bool exist = false;
            if (fs_exists_in_fsdir(files, filename))
                exist = true;
            vector_destroy(files);
            if (exist)
                return pfs_file;
        }
    }
    return nullptr;
}

void submission_package_destroy(submission_package_t *subp) {
    for (size_t i = 0; i < list_size(subp->nca_list); i++) {
        content_archive_destroy(list_get(subp->nca_list, i));
    }
    list_destroy(subp->nca_list);
    pfs_destroy(subp->main_pfs);
    fb_free(subp);
}

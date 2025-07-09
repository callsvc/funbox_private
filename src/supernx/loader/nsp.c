#include <core/types.h>
#include <loader/nsp.h>


bool nsp_is_nsp(fsfile_t *file) {
    if (fs_getsize(file) >= 4096)
        return pfs_is_pfs(file);
    return false;
}

nsp_t *nsp_create(fsfile_t *file, keys_db_t *keys) {
    nsp_t *nsp = funbox_malloc(sizeof(nsp_t));
    nsp->next_loader = submission_package_create(file, keys);
    return nsp;
}
void nsp_destroy(nsp_t *nsp) {
    submission_package_destroy(nsp->next_loader);
    funbox_free(nsp);
}

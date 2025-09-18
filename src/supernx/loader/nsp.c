#include <fs_fmt/pfs.h>
#include <fs_fmt/content_archive.h>

#include <core/types.h>
#include <loader/nsp.h>

bool nsp_is_nsp(fsfile_t *file) {
    if (fs_getsize(file) >= 4096)
        return pfs_is_pfs(file);
    return false;
}

vector_t * nsp_get_logo(const nsp_t * nsp) {
    const romfs_t * logo = (romfs_t*)submission_package_byfile(nsp->nsp_main, "icon_AmericanEnglish.dat", false);
    fsfile_t * logo_file = fs_open_file((fsdir_t*)logo, "icon_AmericanEnglish.dat", "r");

    if (!logo_file)
        return nullptr;
    vector_t *logo_content = fs_getbytes(logo_file, fs_getsize(logo_file), 0);
    fs_close_file((fsdir_t*)logo, logo_file);
    return logo_content;
}
uint64_t nsp_get_program_id(const nsp_t * nsp) {
    const content_archive_t *nca = submission_package_nca_bytype(nsp->nsp_main, content_type_program);
    if (nca && nca->program_id > 0)
        return nca->program_id;
    return 0;
}
vector_t * loader_nsp_get_logo(loader_base_t *base) {
    return nsp_get_logo((nsp_t*)base);
}
uint64_t loader_nsp_get_program_id(loader_base_t *base) {
    return nsp_get_program_id((nsp_t*)base);
}

nsp_t *nsp_create(fsfile_t *file, keys_db_t *keys) {
    nsp_t *nsp = fb_malloc(sizeof(nsp_t));
    nsp->nsp_main = submission_package_create(file, keys);

    nsp->vloader.loader_get_logo = loader_nsp_get_logo;
    nsp->vloader.loader_get_program_id = loader_nsp_get_program_id;
    return nsp;
}
void nsp_destroy(nsp_t *nsp) {
    submission_package_destroy(nsp->nsp_main);
    fb_free(nsp);
}

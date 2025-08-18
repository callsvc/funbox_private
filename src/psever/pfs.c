#include <pfs.h>

#include <types.h>
pfs_t * pfs_create(fsfile_t *pfs_file) {
    pfs_t * pfs = fb_malloc(sizeof(pfs_t));
    pfs->file = pfs_file;

    pfs_superblock_t superblock;
    fs_read(pfs_file, &superblock, sizeof(superblock), 0);

    return pfs;
}
void pfs_destroy(pfs_t *pfs) {
    fb_free(pfs);
}
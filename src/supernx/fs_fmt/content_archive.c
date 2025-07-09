
#include <stdio.h>
#include <types.h>
#include <fs_fmt/aes_file.h>
#include <fs_fmt/offset_file.h>
#include <fs_fmt/content_archive.h>


bool nca_is(const uint32_t magic) {
    const uint32_t nca_versions[] = {*(uint32_t*)"NCA0", *(uint32_t*)"NCA1", *(uint32_t*)"NCA2", *(uint32_t*)"NCA3"};
    for (size_t i = 0; i < 3; i++)
        if (magic == nca_versions[i])
            return true;
    return false;
}

bool is_empty(const uint8_t *p_data, size_t size) {
    for (const size_t len64 = sizeof(size_t); size > len64; size -= len64) {
        if (*(uint64_t*)p_data)
            return false;
        p_data += len64;
    }
    while (size--)
        if (*p_data++)
            return false;
    return true;
}

typedef struct pfs_list_item {
    content_type_e type;
    fsfile_t *file;
    bool aes_encrypted;
} pfs_list_item_t;

pfs_list_item_t * open_decrypted_file(const content_archive_t *nca, const nca_fs_entry_t *this_fs, const nca_fs_header_t * fs_info) {
    pfs_list_item_t * list_item = funbox_malloc(sizeof(pfs_list_item_t));
    list_item->type = nca->type;

    list_item->aes_encrypted = fs_info->enc_type != encryption_type_none;
    const size_t size = (this_fs->end_offset - this_fs->start_offset) * 0x200;

    if (nca->encrypted) {
        const aes_file_t *nca_enc = (aes_file_t*)nca->ncafile;
        list_item->file = (fsfile_t*)offset_file_open(nca_enc->parent, fs_getpath(nca_enc), size, this_fs->start_offset * 0x200);
    } else {
        list_item->file = (fsfile_t*)offset_file_open(nca->ncafile, fs_getpath(nca->ncafile), size, this_fs->start_offset * 0x200);
    }

    return list_item;
}

void content_archive_get_all_files(const content_archive_t *nca, const nca_type_header_t *nca_fs) {
    nca_fs_header_t *nca_fs_info = funbox_malloc(sizeof(nca_fs_header_t));
    for (size_t i = 0; i < NCA_FS_ENTRIES_COUNT; i++) {
        if (is_empty((uint8_t*)&nca_fs->files_entries[i], sizeof(nca_fs_entry_t)))
            continue;

        const nca_fs_entry_t * this_fs = &nca_fs->files_entries[i];
        fs_read(nca->ncafile, nca_fs_info, sizeof(*nca_fs_info), 0x400 + i * 0x200);
        if (nca_fs_info->enc_type == encryption_type_none)
            if (nca_fs_info->type == fs_type_partition_fs) // logofs
                list_push(nca->pfs_list, open_decrypted_file(nca, this_fs, nca_fs_info));

    }
    funbox_free(nca_fs_info);
}

content_archive_t * content_archive_create(keys_db_t *keys, fsdir_t *pfs, const char *path) {
    content_archive_t *nca = funbox_malloc(sizeof(content_archive_t));
    nca->parent_pfs = pfs;
    nca->keys = keys;
    nca->pfs_list = list_create(0);

    nca->ncafile = fs_open_file(pfs, path, "r");

    nca_type_header_t *nca_info = funbox_malloc(sizeof(nca_type_header_t));
    fs_read(nca->ncafile, nca_info, sizeof(*nca_info), 0);
    if (!nca_is(nca_info->magic)) {
        const key256_t *mainkey = keys->header_key;

        aes_file_t *aes_file = aes_file_open(nca->ncafile, aes_type_xts128, "r");
        aes_file_setkey(aes_file, (const uint8_t*)mainkey, sizeof(*mainkey));
        aes_file_asxts(aes_file, 0x200);
        nca->ncafile = (fsfile_t*)aes_file;

        fs_read(nca->ncafile, nca_info, sizeof(*nca_info), 0);

        fprintf(stderr, "this nca at %p is encrypted, header key address: %p\n", nca, mainkey);
        nca->encrypted = true;
    }
    if (nca_info->size != fs_getsize(nca->ncafile))
        oskill("nca size does not match the file size");

    nca->type = nca_info->content_type;
    content_archive_get_all_files(nca, nca_info);
    funbox_free(nca_info);

    return nca;
}

void content_archive_destroy(content_archive_t *nca) {

    for (size_t i = 0; i < list_size(nca->pfs_list); i++) {
        pfs_list_item_t * list_item = list_get(nca->pfs_list, i);
        if (!list_item->aes_encrypted)
            offset_file_close(nca->encrypted ? ((aes_file_t*)nca->ncafile)->parent : nca->ncafile, (offset_file_t*)list_item->file);
        funbox_free(list_item);
    }
    list_destroy(nca->pfs_list);

    if (nca->encrypted) {
        aes_file_t *aes_file = (aes_file_t*)nca->ncafile;
        fs_close_file(nca->parent_pfs, aes_file->parent);

        aes_file_close(aes_file);
    } else {
        fs_close_file(nca->parent_pfs, nca->ncafile);
    }
    funbox_free(nca);
}

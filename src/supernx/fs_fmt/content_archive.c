
#include <stdio.h>
#include <string.h>
#include <types.h>
#include <fs_fmt/pfs.h>
#include <fs_fmt/aes_file.h>
#include <fs_fmt/offset_file.h>
#include <fs_fmt/content_archive.h>



bool nca_is(const uint32_t magic) {
    // ReSharper disable once CppVariableCanBeMadeConstexpr
    const uint32_t nca_versions[] = {*(uint32_t*)"NCA0", *(uint32_t*)"NCA1", *(uint32_t*)"NCA2", *(uint32_t*)"NCA3"};
    for (size_t i = 0; i < 3; i++)
        if (magic == nca_versions[i])
            return true;
    return false;
}

bool is_empty(const uint8_t *p_data, size_t size) {
    for (constexpr size_t len64 = sizeof(size_t); size > len64; size -= len64) {
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
    pfs_t * pfs;
    bool aes_encrypted;
} pfs_list_item_t;

pfs_list_item_t * open_decrypted_file(const content_archive_t *nca, const nca_fs_entry_t *this_fs, const nca_fs_header_t * fs_info, const size_t *file_details) {
    pfs_list_item_t * list_item = fb_malloc(sizeof(pfs_list_item_t));
    list_item->type = nca->type;

    list_item->aes_encrypted = fs_info->enc_type != encryption_type_none;
    size_t size = (this_fs->end_offset - this_fs->start_offset) * 0x200;
    size_t offset = this_fs->start_offset * 0x200;
    if (file_details) {
        offset += file_details[0];
        if (file_details[1] < size)
            size = file_details[1];
    }

    if (nca->encrypted) {
        const aes_file_t *nca_enc = (aes_file_t*)nca->ncafile;
        list_item->file = (fsfile_t*)offset_file_open(nca_enc->parent, fs_getpath(nca_enc), size, offset);
    } else {
        list_item->file = (fsfile_t*)offset_file_open(nca->ncafile, fs_getpath(nca->ncafile), size, offset);
    }

    return list_item;
}

#pragma pack(push, 1)
typedef struct hierarchical_sha256_data {
    uint8_t master_hash[0x20];
    uint32_t block_size;
    uint32_t layer_count;
    struct region {
        uint64_t offset;
        uint64_t size;
    } layer_regions[0x50 / sizeof(struct region)];
    uint8_t _pad0[0x80];
} hashdata_hsd_t;
typedef struct integrity_meta_info {
    uint32_t magic;
    uint32_t version;
    uint32_t master_hash_size;
    struct {
        uint32_t max_layers;
        struct levels {
            uint64_t offset;
            uint64_t hashdata_size;
            uint32_t block_size_ln2;
            uint32_t _pad;
        } levels[0x90 / sizeof(struct levels)];
        uint8_t signature_salt[0x20];
    };
    uint8_t master_hash[0x20];
    uint8_t _pad0[0x18];
} hashdata_imi_t;

#pragma pack(pop)
_Static_assert(sizeof(struct hierarchical_sha256_data) == NCA_FS_HASH_DATA_SIZE);
_Static_assert(sizeof(struct integrity_meta_info) == NCA_FS_HASH_DATA_SIZE);

size_t* content_archive_fix_offsets_for_file(const nca_fs_header_t *file_info) {
    constexpr size_t list_size = sizeof(size_t) * 2;
    size_t * offset_size = fb_malloc(list_size);

    if (file_info->type == fs_type_partition_fs && file_info->hash_type == 2) {
        const hashdata_hsd_t * hashable = (hashdata_hsd_t*)file_info->hash_data;
        if (hashable->layer_count != 2)
            oskill("layout_count must be equal to 2");

        memcpy(offset_size, &hashable->layer_regions[hashable->layer_count - 1], list_size);
    } else if (file_info->type == fs_type_romfs && file_info->hash_type == 3) {
        const hashdata_imi_t * integrity = (hashdata_imi_t*)file_info->hash_data;
        if (integrity->magic != *(uint32_t*)"IVFC")
            oskill("magic value is corrupted");
        const uint64_t max_level = integrity->max_layers - 2;
        memcpy(offset_size, &integrity->levels[max_level], list_size);
    } else {
        fb_free(offset_size);
        offset_size = nullptr;
    }
    return offset_size;
}

void content_archive_get_all_files(const content_archive_t *nca, const nca_type_header_t *nca_fs) {
    nca_fs_header_t *nca_fs_info = fb_malloc(sizeof(nca_fs_header_t));
    for (size_t i = 0; i < NCA_FS_ENTRIES_COUNT; i++) {
        if (is_empty((uint8_t*)&nca_fs->files_entries[i], sizeof(nca_fs_entry_t)))
            continue;

        const nca_fs_entry_t * this_fs = &nca_fs->files_entries[i];
        fs_read(nca->ncafile, nca_fs_info, sizeof(*nca_fs_info), 0x400 + i * 0x200);
        size_t * file_bis = content_archive_fix_offsets_for_file(nca_fs_info);

        if (nca_fs_info->enc_type == encryption_type_none) {
            if (nca_fs_info->type != fs_type_partition_fs) { // logofs
                continue;
            }
            pfs_list_item_t *list_item = open_decrypted_file(nca, this_fs, nca_fs_info, file_bis);
            list_item->pfs = pfs_create(list_item->file);
            if (list_item->pfs)
                list_push(nca->pfs_list, list_item);
        }

        fb_free(file_bis); // can be nullptr in some cases
    }
    fb_free(nca_fs_info);
}

content_archive_t * content_archive_create(keys_db_t *keys, fsdir_t *pfs, const char *path) {
    content_archive_t *nca = fb_malloc(sizeof(content_archive_t));
    nca->parent_pfs = pfs;
    nca->keys = keys;
    nca->pfs_list = list_create(0);

    nca->ncafile = fs_open_file(pfs, path, "r");

    nca_type_header_t *nca_info = fb_malloc(sizeof(nca_type_header_t));
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
    nca->program_id = nca_info->program_id;

    content_archive_get_all_files(nca, nca_info);
    fb_free(nca_info);

    return nca;
}

pfs_t * content_archive_get_pfs(const content_archive_t * nca, const size_t index) {
    if (index > list_size(nca->pfs_list))
        return nullptr;
    const pfs_list_item_t *list_item = list_get(nca->pfs_list, index);
    return list_item->pfs;
}

void content_archive_destroy(content_archive_t *nca) {

    for (size_t i = 0; i < list_size(nca->pfs_list); i++) {
        pfs_list_item_t * list_item = list_get(nca->pfs_list, i);
        if (list_item->pfs)
            pfs_destroy(list_item->pfs);
        if (!list_item->aes_encrypted)
            offset_file_close(nca->encrypted ? ((aes_file_t*)nca->ncafile)->parent : nca->ncafile, (offset_file_t*)list_item->file);
        fb_free(list_item);
    }
    list_destroy(nca->pfs_list);

    if (nca->encrypted) {
        aes_file_t *aes_file = (aes_file_t*)nca->ncafile;
        fs_close_file(nca->parent_pfs, aes_file->parent);

        aes_file_close(aes_file);
    } else {
        fs_close_file(nca->parent_pfs, nca->ncafile);
    }
    fb_free(nca);
}

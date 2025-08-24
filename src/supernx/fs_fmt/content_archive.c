
#include <assert.h>
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

bool is_empty(const uint8_t *data, size_t size) {
    for (constexpr size_t len64 = sizeof(size_t); size > len64; size -= len64) {
        if (*(uint64_t*)data)
            return false;
        data += len64;
    }
    while (size--)
        if (*data++)
            return false;
    return true;
}

typedef struct file_list_item {
    content_type_e type;
    fsfile_t *file;

    bool ispfs;
    union {
        pfs_t * pfs;
        romfs_t *romfs;
    };
    bool aes_encrypted;
} file_list_item_t;

file_list_item_t * open_decrypted_file(const content_archive_t *nca, const nca_fs_entry_t *this_fs, const nca_fs_header_t * fs_info, const size_t *file_details) {
    file_list_item_t * list_item = fb_malloc(sizeof(file_list_item_t));
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
        list_item->file = (fsfile_t*)offset_file_open(nca_enc->parent, fs_getpath(nca_enc), size, offset, false);
    } else {
        list_item->file = (fsfile_t*)offset_file_open(nca->ncafile, fs_getpath(nca->ncafile), size, offset, false);
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
            quit("layout_count must be equal to 2");

        memcpy(offset_size, &hashable->layer_regions[hashable->layer_count - 1], list_size);
    } else if (file_info->type == fs_type_romfs && file_info->hash_type == 3) {
        const hashdata_imi_t * integrity = (hashdata_imi_t*)file_info->hash_data;
        if (integrity->magic != *(uint32_t*)"IVFC")
            quit("magic value is corrupted");
        const uint64_t max_level = integrity->max_layers - 2;
        memcpy(offset_size, &integrity->levels[max_level], list_size);
    } else {
        fb_free(offset_size);
        offset_size = nullptr;
    }
    return offset_size;
}

file_list_item_t * open_encrypted_file(const content_archive_t *nca, const nca_fs_entry_t *this_fs, const nca_fs_header_t * fs_info, const nca_type_header_t *nca_info, const size_t *file_details) {
    uint64_t ctr_values[2] = {};
    ((uint32_t*)&ctr_values)[0] = to_little32(&fs_info->secure_value);
    ((uint32_t*)&ctr_values)[1] = to_little32(&fs_info->generation);

    file_list_item_t * file_item = fb_malloc(sizeof(file_list_item_t));
    file_item->aes_encrypted = true;
    file_item->type = nca->type;

    aes_file_t *aes_file = aes_file_open(((const aes_file_t*)nca->ncafile)->parent, aes_type_ctr128, "r");
    aes_file_setiv(aes_file, (uint8_t*)ctr_values);
    if (is_empty((const uint8_t*)&nca->rights_id, sizeof(nca->rights_id))) {
        key128_t dec_key = {};
        keys_getkey_fornca(nca->keys, &dec_key, sizeof(dec_key), fs_info, nca_info);

        aes_file_setkey(aes_file, (const uint8_t*)&dec_key, sizeof(dec_key));
    } else {
        key128_t title_key = {};
        keys_getkey_fromrights(nca->keys, &title_key, sizeof(title_key), (const uint8_t*)&nca->rights_id, nca_info);
        aes_file_setkey(aes_file, (const uint8_t*)&title_key, sizeof(title_key));
    }
    aes_file_setconstraints(aes_file, 0x200, this_fs->start_offset + (*file_details / 0x200), *file_details % 0x200, this_fs->end_offset);

    file_item->file = (fsfile_t*)aes_file;
#if 1
    uint8_t buffer[4096] = {};
    fs_read(file_item->file, buffer, 16, 0);
#endif

    return file_item;
}

void content_archive_get_all_files(const content_archive_t *nca, const nca_type_header_t *nca_fs) {
    nca_fs_header_t *nca_fs_info = fb_malloc(sizeof(nca_fs_header_t));
    for (size_t i = 0; i < NCA_FS_ENTRIES_COUNT; i++) {
        if (is_empty((uint8_t*)&nca_fs->files_entries[i], sizeof(nca_fs_entry_t)))
            continue;

        const nca_fs_entry_t * this_fs = &nca_fs->files_entries[i];
        fs_read(nca->ncafile, nca_fs_info, sizeof(*nca_fs_info), 0x400 + i * 0x200);
        size_t * file_bis = content_archive_fix_offsets_for_file(nca_fs_info);

        file_list_item_t *list_item = nullptr;
        switch (nca_fs_info->enc_type) {
            case encryption_type_none:
                list_item = open_decrypted_file(nca, this_fs, nca_fs_info, file_bis); break;
            case encryption_type_aes_xts:
                quit("missing for");
            case encryption_type_aes_ctr:
            case encryption_type_aes_ctr_ex:
                list_item = open_encrypted_file(nca, this_fs, nca_fs_info, nca_fs, file_bis); break;
            default:
        }
        if (list_item && nca_fs_info->type == fs_type_partition_fs) { // logofs
            if (!pfs_is_pfs(list_item->file))
                quit("isn't a pfs file");
            list_item->pfs = pfs_create(list_item->file);
            list_item->ispfs = true;
            if (list_item->pfs)
                list_push(nca->pfs_list, list_item);
        } else if (list_item && nca_fs_info->type == fs_type_romfs) {
            list_item->romfs = romfs_create(list_item->file);
            list_push(nca->romfs_list, list_item);
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
    nca->romfs_list = list_create(0);

    nca->ncafile = fs_open_file(pfs, path, "r");

    nca_type_header_t *nca_info = fb_malloc(sizeof(nca_type_header_t));
    fs_read(nca->ncafile, nca_info, sizeof(*nca_info), 0);
    if (!nca_is(nca_info->magic)) {
        const key256_t *mainkey = keys->header_key;

        aes_file_t *aes_file = aes_file_open(nca->ncafile, aes_type_xts128, "r");
        aes_file_setkey(aes_file, (const uint8_t*)mainkey, sizeof(*mainkey));
        aes_file_setconstraints(aes_file, 0x200, 0, 0, 0);
        nca->ncafile = (fsfile_t*)aes_file;

        fs_read(nca->ncafile, nca_info, sizeof(*nca_info), 0);

        logger_info("this nca at %p is encrypted, header key address: %p", nca, mainkey);
        nca->encrypted = true;
    }
    if (nca_info->size != fs_getsize(nca->ncafile))
        quit("nca size does not match the file size");

    nca->type = nca_info->content_type;
    nca->program_id = nca_info->program_id;
    memcpy(&nca->rights_id, &nca_info->rights_id, sizeof(nca_info->rights_id));

    content_archive_get_all_files(nca, nca_info);
    fb_free(nca_info);

    return nca;
}

pfs_t * content_archive_get_pfs(const content_archive_t * nca, const size_t index) {
    if (index > list_size(nca->pfs_list))
        return nullptr;
    const file_list_item_t *list_item = list_get(nca->pfs_list, index);
    return list_item->pfs;
}

void nca_destroy_files(const content_archive_t *nca, const list_t *files) {
    for (size_t i = 0; i < list_size(files); i++) {
        file_list_item_t * list_item = list_get(files, i);
        if (list_item->pfs && list_item->ispfs)
            pfs_destroy(list_item->pfs);
        else if (list_item->romfs)
            romfs_destroy(list_item->romfs);

        if (!list_item->aes_encrypted)
            offset_file_close(nca->encrypted ? ((aes_file_t*)nca->ncafile)->parent : nca->ncafile, (offset_file_t*)list_item->file);
        else aes_file_close((aes_file_t*)list_item->file);
        fb_free(list_item);
    }
}

void content_archive_destroy(content_archive_t *nca) {
    nca_destroy_files(nca, nca->pfs_list);
    nca_destroy_files(nca, nca->romfs_list);
    list_destroy(nca->pfs_list);
    list_destroy(nca->romfs_list);

    if (nca->encrypted) {
        const auto aes_file = (aes_file_t*)nca->ncafile;
        fs_close_file(nca->parent_pfs, aes_file->parent);

        aes_file_close(aes_file);
    } else {
        fs_close_file(nca->parent_pfs, nca->ncafile);
    }
    fb_free(nca);
}

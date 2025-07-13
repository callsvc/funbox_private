#pragma once
#include <horizon/keys_db.h>
#include <fs/types.h>

typedef enum distribution_type : uint8_t {
    distribution_type_download,
    distribution_type_gamecard,
} distribution_type_e;
typedef enum content_type : uint8_t {
    content_type_program,
    content_type_meta,
    content_type_control,
    content_type_manual,
    content_type_data,
    content_type_public_data
} content_type_e;

typedef enum key_area_index_type : uint8_t {
    key_area_index_application,
    key_area_index_ocean,
    key_area_index_system
} key_area_index_type_e;

typedef enum key_generation : uint8_t {
    key_generation_v3_0_1 = 0x03,
    key_generation_v4_0_0,
    key_generation_v5_0_0,
    key_generation_v6_0_0,
    key_generation_v6_2_0,
    key_generation_v7_0_0,
    key_generation_v8_1_0,
    key_generation_v9_0_0,
    key_generation_v9_1_0,
    key_generation_v12_1_0,
    key_generation_v13_0_0,
    key_generation_v14_0_0,
    key_generation_v15_0_0,
    key_generation_v16_0_0,
    key_generation_v17_0_0,
    key_generation_v18_0_0,
    key_generation_v19_0_0,
    key_generation_v20_0_0,
    key_generation_invalid = 0xFF
} key_generation_e;

typedef enum fs_type : uint8_t {
    fs_type_romfs,
    fs_type_partition_fs // pfs0
} fs_type_e;

typedef enum encryption_type : uint8_t {
    encryption_type_auto,
    encryption_type_none,
    encryption_type_aes_xts,
    encryption_type_aes_ctr,
    encryption_type_aes_ctr_ex,
} encryption_type_e;

#define NCA_FS_HASH_DATA_SIZE 0xF8
#pragma pack(push, 1)
typedef struct nca_fs_header {
    uint16_t version;
    fs_type_e type;
    uint8_t hash_type;
    encryption_type_e enc_type;
    uint8_t metadata_hash_type;
    uint16_t _pad0;
    uint8_t hash_data[NCA_FS_HASH_DATA_SIZE];
    uint8_t patch_info[0x40];
    uint32_t generation;
    uint32_t secure_value;
    uint8_t sparce_info[0x30];

    uint8_t _pad1[0x28 + 0x30 + 0x30];
} nca_fs_header_t;

_Static_assert(sizeof(nca_fs_header_t) == 0x200);

typedef struct nca_fs_entry {
    uint32_t start_offset;
    uint32_t end_offset;
    uint64_t _pad0;
} nca_fs_entry_t;


#define NCA_FS_ENTRIES_COUNT 4
typedef struct nca_type_header {
    uint8_t sign_fixed_key[0x100];
    uint8_t sign_npdm_key[0x100];
    uint32_t magic;
    distribution_type_e dist_type;
    content_type_e content_type;
    uint8_t key_gen_old;
    key_area_index_type_e keyarea_type;
    size_t size;
    uint64_t program_id;
    uint32_t content_index;
    uint32_t sdk_addon_version;
    key_generation_e key_gen;
    uint8_t signature_key_gen;
    uint8_t _pad0[0xE];
    key128_t rights_id;

    nca_fs_entry_t files_entries[NCA_FS_ENTRIES_COUNT];
    uint8_t array_of_hashes[0x20 * NCA_FS_ENTRIES_COUNT];
    uint8_t encrypted_key_area[0x10 * NCA_FS_ENTRIES_COUNT];

    uint8_t _pad1[1024 - 832];
} nca_type_header_t;

_Static_assert(sizeof(nca_type_header_t) == 1024);
#pragma pack(pop)

typedef struct content_archive {
    fsdir_t *parent_pfs;
    fsfile_t *ncafile;
    keys_db_t *keys;
    bool encrypted;
    content_type_e type;

    uint64_t program_id;
    list_t *pfs_list;
} content_archive_t;


content_archive_t * content_archive_create(keys_db_t*, fsdir_t *, const char *);
pfs_t * content_archive_get_pfs(const content_archive_t*, size_t);
void content_archive_destroy(content_archive_t *);
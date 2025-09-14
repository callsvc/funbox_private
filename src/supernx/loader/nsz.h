#pragma once

#include <fs/types.h>
#include <loader/types.h>

#include <horizon/keys_db.h>

typedef struct nsz {
    loader_base_t vloader;
    pfs_t * main_pfs;
    romfs_t *nca_files;
    list_t *nca_list;

} nsz_t;

typedef struct ncz_section {
    uint64_t offset;
    uint64_t size;
    uint64_t crypto_type;
    uint64_t pad0;
    uint8_t crypto_key[16];
    uint8_t crypto_iv[16];
} ncz_section_t;
typedef struct ncz_block {
    uint64_t magic;

    uint8_t version;
    uint8_t type;
    uint8_t pad0;
    uint8_t blksize_exp;
    uint64_t pad;
    uint32_t blocks_count;
    uint64_t block_size;
} ncz_block_t;


typedef struct ncz_stream {
    size_t nca_size;
    ncz_section_t *sections;
    size_t sections_size;
    ncz_block_t * blocks;
    size_t blocks_size;
} ncz_t;

nsz_t * nsz_create(fsfile_t *, keys_db_t *);
void nsz_destroy(nsz_t *);
bool nsz_is_nsz(fsfile_t*);

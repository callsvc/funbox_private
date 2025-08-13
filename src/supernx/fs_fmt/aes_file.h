#pragma once
#include <mbedtls/cipher.h>

#include <algo/vector.h>
#include <fs/types.h>

typedef enum aes_type {
    aes_type_ctr128,
    aes_type_xts128,
} aes_type_e;

typedef struct aes_file {
    fsfile_t vfile;
    fsfile_t *parent;

    char mode[2];

    uint64_t sector_size;
    uint64_t sector_offset, sector_pad;
    uint64_t sector_last;
    mbedtls_cipher_context_t context;
    uint8_t iv_ctr[16];
    vector_t *buffer;
} aes_file_t;

aes_file_t * aes_file_open(fsfile_t*, aes_type_e, const char*);
void aes_file_setkey(aes_file_t *, const uint8_t *, size_t);
void aes_file_setiv(aes_file_t *, uint8_t[0x10]);
void aes_file_setconstraints(aes_file_t *, uint64_t, uint64_t, uint64_t, uint64_t);

void aes_file_close(aes_file_t *);

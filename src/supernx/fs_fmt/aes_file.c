#include <assert.h>
#include <string.h>
#include <types.h>
#include <fs_fmt/aes_file.h>

bool aes_file_isxts(const aes_file_t *aes_file) {
    const mbedtls_cipher_type_t type = mbedtls_cipher_get_type(&aes_file->context);
    if (type != MBEDTLS_CIPHER_AES_128_XTS &&
        type != MBEDTLS_CIPHER_AES_256_XTS)
        return false;
    return true;
}

uint8_t * aes_get_nintendo_tweak(const uint64_t sector) {
    static _Thread_local uint8_t iv_buffer[0x10] = {};

    const uint64_t le_sector = to_little64(&sector);
    memcpy(iv_buffer + 8, &le_sector, sizeof(le_sector));
    return iv_buffer;
}


size_t roundto(const size_t value, const size_t n) {
    return (value + n) & ~(n);
}
void aes_file_update(aes_file_t *aes_file, void *output, const size_t size, const size_t offset) {
    const size_t block_size = mbedtls_cipher_get_block_size(&aes_file->context);

    if (size > vector_size(aes_file->buffer))
        vector_setsize(aes_file->buffer, roundto(size, block_size));

    uint8_t *dest_buffer = vector_begin(aes_file->buffer);
    if (strchr(aes_file->mode, 'r'))
        fs_read(aes_file->parent, output, size, offset);
    mbedtls_cipher_reset(&aes_file->context);

    size_t result = 0;
    if (unlikely(!aes_file_isxts(aes_file))) {
        size_t i = 0;
        for (; i < size; i += result)
            if (mbedtls_cipher_update(&aes_file->context, output + i, MIN(size - i, 0x10), dest_buffer + i, &result))
                if (result != block_size)
                    quit("can't update block");
        result = i;
    } else {
        mbedtls_cipher_update(&aes_file->context, output, size, dest_buffer, &result);

    }
    if (result < size)
        quit("can't update all blocks");
    memcpy(output, dest_buffer, size);
}

void aes_file_read_xts(aes_file_t *aes_file, void *output, const size_t size, size_t offset) {
    if (offset && offset % aes_file->sector_size)
        return;
    const size_t sector_end = offset + size;

    uint8_t *p_out = output;
    const size_t secsize = aes_file->sector_size;

    size_t tweak = offset ? offset / secsize : 0;
    for (size_t it_offset = 0; it_offset <= sector_end - offset; ) {
        aes_file_setiv(aes_file, aes_get_nintendo_tweak(tweak));
        aes_file_update(aes_file, p_out + it_offset, secsize, offset);
        offset += secsize;
        it_offset += secsize;

        tweak++;
    }
}


void aes_file_updatectr(aes_file_t *aes_file, const size_t offset) {
    const uint64_t blkoffset = offset >> 4;
    const uint64_t le_offset = to_little64(&blkoffset);
    memcpy(aes_file->iv_ctr + 8, &le_offset, 8);
    aes_file_setiv(aes_file, aes_file->iv_ctr);
}

constexpr size_t ctr_block_size = 0x10;
void aes_file_read_ctr(aes_file_t *aes_file, void *output, const size_t size, const size_t offset) {
    const size_t alignoffset = offset % ctr_block_size;

    const size_t padding = ctr_block_size - alignoffset;
    uint8_t start_block[ctr_block_size];
    size_t filled = 0;
    if (!alignoffset) {
        aes_file_updatectr(aes_file, offset);
        aes_file_update(aes_file, output, size, offset);
    } else {
        aes_file_updatectr(aes_file, offset - alignoffset);
        aes_file_update(aes_file, start_block, ctr_block_size, offset - alignoffset);
        if (size + alignoffset <= ctr_block_size) {
            memcpy(output, &start_block[alignoffset], size);
        } else {
            memcpy(output, &start_block[alignoffset], padding);
            filled += padding;
        }
    }

    if (filled)
        aes_file_read_ctr(aes_file, (uint8_t*)output + filled, size - filled, offset + filled);
}

void fs_aes_file_read(fsfile_t *file, void *output, const size_t size, const size_t offset) {
    const auto aes_file = (aes_file_t*)file;
    const uint64_t offset_block = aes_file->sector_offset * aes_file->sector_size + aes_file->sector_pad + offset;

    if (aes_file_isxts((aes_file_t*)file))
        aes_file_read_xts((aes_file_t*)file, output, size, offset_block);
    else aes_file_read_ctr((aes_file_t*)file, output, size, offset_block);
}
void fs_aes_file_write(fsfile_t *file, const void *input, const size_t size, const size_t offset) {
    const auto aes_file = (aes_file_t*)file;

    assert(size % ctr_block_size == 0 && offset % ctr_block_size == 0);
    if (input == nullptr) {
        if (size > vector_size(aes_file->enc_buffer))
            vector_setsize(aes_file->enc_buffer, roundto(size, ctr_block_size));
        input = vector_begin(aes_file->enc_buffer);
        fs_read(aes_file->parent, (void*)input, size, offset);
    }
    aes_file_updatectr(aes_file, offset);
    aes_file_update(aes_file, (void*)input, size, offset);
    fs_write(aes_file->parent, input, size, offset);
}

size_t fs_aes_file_getsize(const fsfile_t *file) {
    const aes_file_t * aes_file = (aes_file_t*)file;
    return fs_getsize(aes_file->parent);
}

aes_file_t * aes_file_open(fsfile_t * file, const aes_type_e type, const char *mode) {
    aes_file_t * aes_file = fb_malloc(sizeof(aes_file_t));
    aes_file->parent = file;

    strcpy(aes_file->mode, mode);
    strcpy(aes_file->vfile.path, fs_getpath(file));
    aes_file->vfile.type = file_type_aes;
    aes_file->buffer = vector_create(0, sizeof(uint8_t));

    if (strchr(mode, 'w'))
        aes_file->enc_buffer = vector_create(0, sizeof(uint8_t));

    aes_file->vfile.fs_read = fs_aes_file_read;
    aes_file->vfile.fs_write = fs_aes_file_write;
    aes_file->vfile.fs_getsize = fs_aes_file_getsize;

    mbedtls_cipher_init(&aes_file->context);
    const mbedtls_cipher_info_t * info = nullptr;
    if (type == aes_type_ctr128)
        info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_CTR);
    else if (type == aes_type_xts128)
        info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_XTS);

    mbedtls_cipher_setup(&aes_file->context, info);

    return aes_file;
}

void aes_file_setkey(aes_file_t *aes_file, const uint8_t *key, const size_t keylen) {
    const size_t len = mbedtls_cipher_get_key_bitlen(&aes_file->context);
    if (len == MBEDTLS_KEY_LENGTH_NONE || len != keylen * 8)
        quit("invalid key size");
    const mbedtls_operation_t mbedtls_op = *aes_file->mode == 'r' ? MBEDTLS_DECRYPT : MBEDTLS_ENCRYPT;

    if (mbedtls_cipher_setkey(&aes_file->context, key, (int)len, mbedtls_op))
        quit("can't set key");
}

void aes_file_setiv(aes_file_t *aes_file, const uint8_t iv[16]) {
    assert(mbedtls_cipher_set_iv(&aes_file->context, iv, 0x10) == 0);
    memmove(aes_file->iv_ctr, iv, 16);
}

void aes_file_setconstraints(aes_file_t *aes_file, const uint64_t size, const uint64_t offset, const size_t padding, const uint64_t end_offset) {
    aes_file->sector_size = size;
    aes_file->sector_offset = offset;
    aes_file->sector_pad = padding;

    aes_file->sector_last = end_offset;

    const size_t last_offset = aes_file->sector_last * aes_file->sector_size;
    if (fs_getsize(aes_file->parent) < last_offset)
        quit("invalid offset");
}

void aes_file_close(aes_file_t *aes_file) {
    mbedtls_cipher_free(&aes_file->context);
    if (aes_file->buffer)
        vector_destroy(aes_file->buffer);
    if (aes_file->enc_buffer)
        vector_destroy(aes_file->enc_buffer);
    fb_free(aes_file);
}

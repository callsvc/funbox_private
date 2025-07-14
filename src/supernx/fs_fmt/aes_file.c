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

    const uint64_t be_sector = __builtin_bswap64(sector);
    memcpy(&iv_buffer[sizeof(uint64_t)], &be_sector, sizeof(be_sector));
    return iv_buffer;
}

void aes_file_update(aes_file_t *aes_file, void *output, const size_t size, const size_t offset) {
    const size_t block_size = mbedtls_cipher_get_block_size(&aes_file->context);

    if (size > vector_size(aes_file->buffer))
        vector_setsize(aes_file->buffer, size);

    uint8_t *input = vector_begin(aes_file->buffer);
    fs_read(aes_file->parent, input, size, offset);
    mbedtls_cipher_reset(&aes_file->context);

    size_t result = 0;
    if (!aes_file_isxts(aes_file)) {
        for (size_t i = 0; i < size; i += result)
            if (mbedtls_cipher_update(&aes_file->context, input, vector_size(aes_file->buffer), output, &result))
                if (result != block_size)
                    oskill("can't update block");
    }

    mbedtls_cipher_update(&aes_file->context, input, vector_size(aes_file->buffer), output, &result);
    if (result != size)
        oskill("can't update all blocks");
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

static constexpr size_t ctr_block_size = 0x10;
void aes_file_read_ctr_misaligned(aes_file_t *aes_file, void *output, const size_t size, size_t offset) {
    const size_t alignoffset = offset % ctr_block_size;

    const size_t padding = ctr_block_size - alignoffset;
    if (!alignoffset) {
        aes_file_setiv(aes_file, aes_get_nintendo_tweak(offset / ctr_block_size));
        aes_file_update(aes_file, output, ctr_block_size, offset);
    } else {
        uint8_t start_block[ctr_block_size];
        aes_file_update(aes_file, start_block, ctr_block_size, offset - alignoffset);
        if (size + alignoffset < ctr_block_size) {
            memcpy(output, &start_block[alignoffset], size);
            return;
        }
        memcpy(output, &start_block[alignoffset], padding);
    }

    const size_t fill_size = size - padding;
    if (fill_size)
        aes_file_read_ctr_misaligned(aes_file, (uint8_t*)output + fill_size, fill_size, offset + padding);
}

void fs_aes_file_read(fsfile_t *file, void *output, const size_t size, const size_t offset) {
    if (aes_file_isxts((aes_file_t*)file))
        aes_file_read_xts((aes_file_t*)file, output, size, offset);
    else aes_file_read_ctr_misaligned((aes_file_t*)file, output, size, offset);
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

    aes_file->vfile.fs_read = fs_aes_file_read;
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
        return;
    const mbedtls_operation_t mbedtls_op = *aes_file->mode == 'r' ? MBEDTLS_DECRYPT : MBEDTLS_ENCRYPT;

    mbedtls_cipher_setkey(&aes_file->context, key, len, mbedtls_op);
}

void aes_file_setiv(aes_file_t *aes_file, uint8_t iv[16]) {
    mbedtls_cipher_set_iv(&aes_file->context, iv, 0x10);
}

void aes_file_asxts(aes_file_t *aes_file, const uint64_t sector_size) {
    if (!aes_file_isxts(aes_file))
        oskill("not a xts file");

    aes_file->sector_size = sector_size;
}

void aes_file_close(aes_file_t *aes_file) {
    mbedtls_cipher_free(&aes_file->context);
    if (aes_file->buffer)
        vector_destroy(aes_file->buffer);
    fb_free(aes_file);
}

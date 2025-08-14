#include <assert.h>
#include <stdio.h>
#include <string.h>

#include <mbedtls/cipher.h>

#include <types.h>
#include <fs_fmt/content_archive.h>
#include <horizon/keys_db.h>

int32_t get_generation(const nca_type_header_t *nca_info) {
    int32_t gen_old = MAX(nca_info->key_gen_old, nca_info->key_gen);
    if (gen_old > 0)
        gen_old--;
    return gen_old;
}

void mbedtls_fast_ecbdec(const uint8_t *src, const size_t src_size, const uint8_t *key, const size_t key_size, uint8_t *dest, size_t size) {
    mbedtls_cipher_context_t context;
    mbedtls_cipher_init(&context);
    mbedtls_cipher_setup(&context, mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_128_ECB));

    assert(mbedtls_cipher_setkey(&context, key, key_size * 8, MBEDTLS_DECRYPT) == 0);

    mbedtls_cipher_reset(&context);
    mbedtls_cipher_update(&context, src, src_size, dest, &size);
    mbedtls_cipher_free(&context);
}
void keys_getkey_fromrights(const keys_db_t * keys, void * dest, const size_t size, const uint8_t *rights, const nca_type_header_t *nca_info) {
    key128_t title = {};
    keys_db_get_titlekey(keys, &title, (const key128_t*)rights);

    char titlekek_name[24] = {};
    sprintf(titlekek_name, "titlekek_%02u", get_generation(nca_info));
    if (ht_contains(keys->tag_keysmap, titlekek_name)) {
        const tagged_key_t * key = ht_get(keys->tag_keysmap, titlekek_name);
        mbedtls_fast_ecbdec((uint8_t*)&title, 0x10, (const uint8_t*)&key->indexed, 0x10, dest, size);
    } else {

        quit("kek not found!!!");
    }

}



void keys_getkey_fornca(const keys_db_t *kdb, void *dest, const size_t size, const nca_fs_header_t *this_fs, const nca_type_header_t *nca_info) {
    size_t key_index = 0;
    switch (this_fs->enc_type) {
        case encryption_type_aes_ctr:
        case encryption_type_aes_ctr_ex:
            key_index = 2;
        default:
    }
    char keyarea_name[38] = {};
    const char *key_name = nullptr;
    switch (nca_info->keyarea_type) {
        case key_area_index_application:
            key_name = "application"; break;
        case key_area_index_ocean:
            key_name = "ocean"; break;
        case key_area_index_system:
            key_name = "system"; break;
        default:
    }


    sprintf(keyarea_name, "key_area_key_%s_%02u", key_name, get_generation(nca_info));
    if (!ht_contains(kdb->tag_keysmap, keyarea_name))
        return;

    const tagged_key_t *this_key = ht_get(kdb->tag_keysmap, keyarea_name);
    mbedtls_fast_ecbdec(&nca_info->encrypted_key_area[key_index * 0x10], 0x10, (const uint8_t*)&this_key->indexed, 0x10, dest, size);

}

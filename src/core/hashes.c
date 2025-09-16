#include <string.h>
#include <mbedtls/sha1.h>
#include <mbedtls/sha256.h>

#include <fs/types.h>
#include <types.h>


bool cmpsha(const uint8_t *bytes, const size_t size, const char *sha) {
    mbedtls_sha1_context sha1;
    mbedtls_sha1_init(&sha1);

    mbedtls_sha1_starts(&sha1);
    mbedtls_sha1_update(&sha1, bytes, size);

    uint8_t result[20];
    mbedtls_sha1_finish(&sha1, result);

    uint8_t against[20];
    strtobytes(sha, against, 20);
    mbedtls_sha1_free(&sha1);

    return memcmp(result, against, sizeof(result)) == 0;
}

void fs_sha256(fsfile_t *file, uint8_t out[256 / 8]) {
    mbedtls_sha256_context sha2;
    mbedtls_sha256_init(&sha2);
    mbedtls_sha256_starts(&sha2, 0);

    uint8_t buffer[64 * 1024];
    for (size_t i = 0; i < fs_getsize(file); ) {
        const size_t rsize = MIN(fs_getsize(file) - i, sizeof(buffer));
        fs_read(file, buffer, rsize, i);

        mbedtls_sha256_update(&sha2, buffer, rsize);
        i += rsize;
    }
    mbedtls_sha256_finish(&sha2, out);
    mbedtls_sha256_free(&sha2);
}
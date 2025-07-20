#include <string.h>
#include <mbedtls/sha1.h>

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
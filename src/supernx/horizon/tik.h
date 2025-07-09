#pragma once
#include <stdbool.h>
#include <fs/types.h>

typedef enum signature_type {
    rsa_4096_pkcs_sha1 = 0x010000,
    rsa_2048_pkcs_sha1,
    ecdsa_sha1,
    rsa_4096_pkcs_sha2,
    rsa_2048_pkcs_sha2,
    ecdsa_sha2,
    hmac_sha1,
} signature_type_e;

#pragma pack(push, 1)
typedef struct ticket_data {
    char issuer[0x40];
    uint8_t title_key_block[0x100];
    uint8_t ticket_version;
    uint8_t ticket_key_type;
    uint16_t ticket_version_2;
    uint8_t license_type;
    uint8_t master_key_rev;
    uint16_t properties_bitfield;
    uint64_t _pad0;
    uint64_t ticket_id;
    uint64_t device_id;
    uint8_t rights_id[0x10];
    uint32_t account_id;
    uint8_t _pad1[0xC];


} ticket_data_t;
#pragma pack(pop)

typedef struct tik {
    signature_type_e type;
    vector_t *signature;

    ticket_data_t ticket_data;
} tik_t;

tik_t * tik_create(fsfile_t *);
void tik_destroy(tik_t *);
bool tik_isequal(const tik_t *, const tik_t *);

void tik_export(const tik_t *, const char *);

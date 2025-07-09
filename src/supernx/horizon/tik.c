#include <stdio.h>
#include <string.h>
#include <fs/file.h>
#include <types.h>

#include <horizon/tik.h>
tik_t * tik_create(fsfile_t *file) {
    tik_t * tik = funbox_malloc(sizeof(tik_t));

    fs_read(file, &tik->type, sizeof(tik->type), 0);
    uint64_t offset = sizeof(tik->type);

    uint32_t size = 0;
    switch (tik->type) {
        case rsa_4096_pkcs_sha1:
        case rsa_4096_pkcs_sha2:
            size = 0x23C; break;
        case rsa_2048_pkcs_sha1:
        case rsa_2048_pkcs_sha2:
            size = 0x13C; break;
        case ecdsa_sha1:
        case ecdsa_sha2:
            size = 0x3C + 0x40; break;
        case hmac_sha1:
            size = 0x14 + 0x28; break;

    }
    tik->signature = fs_getbytes(file, size, offset);
    offset += vector_size(tik->signature);
    fs_read(file, &tik->ticket_data, sizeof(tik->ticket_data), offset);

    ticket_data_t *tik_content = &tik->ticket_data;
    fprintf(stderr, "ticket issuer entity: %s\n", tik_content->issuer);

    return tik;
}
void tik_destroy(tik_t *tik) {
    vector_destroy(tik->signature);
    funbox_free(tik);
}

bool tik_isequal(const tik_t *tika, const tik_t *tikb) {
    if (tika->type == tikb->type)
        if (memcmp(&tika->ticket_data, &tikb->ticket_data, sizeof(tika->ticket_data)) == 0)
            if (vector_isequal(tika->signature, tikb->signature))
                return true;
    return false;
}

void tik_export(const tik_t *tik, const char *filepath) {
    file_t *file = file_open(filepath, "w");
    (void)tik->type;
    file_close(file);
}

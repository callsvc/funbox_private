#pragma once
#include <regex.h>

#include <algo/list.h>
#include <algo/set.h>
#include <algo/ht.h>


typedef struct key128 {
    uint8_t data[16];
} key128_t;
typedef struct key256 {
    uint8_t data[32];
} key256_t;

// typedef uint8_t key128_t[0x10];
// typedef uint8_t key256_t[0x20];

typedef enum key_type {
    key_none,
    key_titlekek,
    key_area_application,
    key_area_ocean,
    key_area_system,

} key_type_e;

typedef struct tagged_key {
    key_type_e type;
    uint16_t index;


    union {
        key256_t value;
        key128_t indexed;
    };
} tagged_key_t;

typedef struct keys_db {
    list_t * keys_path;
    list_t * tickets;
    set_t * titles;


    const key256_t *header_key;
    ht_t *named_keys;
    ht_t *tag_keysmap;

    regex_t prod_regex;
    regex_t title_regex;

    size_t count;
} keys_db_t;

keys_db_t *keys_db_create();
void keys_db_load(keys_db_t*, fsfile_t *);

typedef struct tik tik_t;
void keys_db_add_ticket(const keys_db_t*, const tik_t*);
void keys_db_get_titlekey(const keys_db_t*, key128_t*, const key128_t*);

void keys_db_destroy(keys_db_t *);


typedef struct nca_fs_header nca_fs_header_t;
typedef struct nca_type_header nca_type_header_t;
void keys_getkey_fornca(const keys_db_t *, void *, size_t, const nca_fs_header_t *, const nca_type_header_t *);
void keys_getkey_fromrights(const keys_db_t *, void *, size_t, const uint8_t *, const nca_type_header_t *);
#pragma once

#include <stdint.h>
#include <algo/vector.h>

#define KEY_TYPE_NAME_SIZE 0x50
#define KEY_TYPE_VALUE_SIZE sizeof(uint64_t)
#define KEY_TYPE_POINTER_SIZE sizeof(void *)

typedef struct robin_map_entry {
    union {
        char keyname[KEY_TYPE_NAME_SIZE];
        uint64_t keyval;
        void * keyptr;
    };
    union {
        char valname[KEY_TYPE_NAME_SIZE];
        uint64_t valval;
        void * valptr;
    };

    size_t psl_value;
    bool filled;
} robin_map_entry_t;

typedef struct robin_map {
    vector_t * bucket;
    uint8_t secret_key[16];

    uint8_t pair_types[2];
} robin_map_t;

robin_map_t * robin_map_create(uint8_t[2]);

typedef bool (*robin_callback_t)(robin_map_entry_t *, void *);

void robin_map_emplace(robin_map_t *, const void*, const void*);
void robin_map_foreach(const robin_map_t *, robin_callback_t, void *);
void * robin_map_get(const robin_map_t *, const void *);
void * robin_map_gethash(const robin_map_t *, const void *, uint64_t hash);

void robin_map_print(const robin_map_t*);
void robin_map_destroy(robin_map_t *);
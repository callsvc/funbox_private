#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <types.h>

#include <algo/robin.h>

uint64_t siphash(const void *input, size_t len, const uint8_t key[16]) {
#define SIP_SHIFT(a, b) (uint64_t)(((a) << (b)) | ((a) >> (64 - (b))))
#define SIP_ROTATE(a, b, c, d)\
    do {\
        a += b; b = SIP_SHIFT(b, 13); b ^= a; a = SIP_SHIFT(a, 32);\
        c += d; d = SIP_SHIFT(d, 16); d ^= c;\
        a += d; d = SIP_SHIFT(d, 21); d ^= a;\
        c += b; b = SIP_SHIFT(b, 17); b ^= c; c = SIP_SHIFT(c, 32);\
   } while (false)

#define SIP_DATA_ROTATE(a, b, c, d, val)\
    d ^= val;\
    SIP_ROTATE(v0, v1, v2, v3);\
    SIP_ROTATE(v0, v1, v2, v3);\
    a ^= val

    // C = 2, D = 4
    const uint64_t *key_words = (uint64_t*)key;
    uint64_t v0 = key_words[0] ^ 0x736F6D6570736575;
    uint64_t v1 = key_words[1] ^ 0x646F72616E646F6D;
    uint64_t v2 = key_words[0] ^ 0x6C7967656E657261;
    uint64_t v3 = key_words[1] ^ 0x7465646279746573;

    uint64_t mval = 0;
    size_t index = 0;
    for (; len - index >= 8; index += 8) {
        mval = *(uint64_t *)((uint8_t*)input + index);
        SIP_DATA_ROTATE(v0, v1, v2, v3, mval);
    }

    mval = len << 56;
    for (size_t i = 0; i < (len & 7); ++i)
        mval |= (uint64_t)((uint8_t*)input)[index + i] << (8 * i);
    SIP_DATA_ROTATE(v0, v1, v2, v3, mval);

    v2 ^= 0xFF;
    SIP_ROTATE(v0, v1, v2, v3);
    SIP_ROTATE(v0, v1, v2, v3);
    SIP_ROTATE(v0, v1, v2, v3);
    SIP_ROTATE(v0, v1, v2, v3);

    return v0 ^ v1 ^ v2 ^ v3;
}

robin_map_t * robin_map_create(uint8_t types[2]) {
    robin_map_t * robin_map = fb_malloc(sizeof(robin_map_t));

    robin_map->bucket = vector_create(0, sizeof(robin_map_entry_t));
    memcpy(robin_map->pair_types, types, sizeof(uint8_t) * 2);

    const uint64_t a = fb_rand();
    const uint64_t b = fb_rand();
    memcpy(robin_map->secret_key, &a, 8);
    memcpy(&robin_map->secret_key[8], &b, 8);

    return robin_map;
}

size_t robin_pair_type_size(const uint8_t type) {
    if (type == 1)
        return KEY_TYPE_NAME_SIZE;
    if (type == 2)
        return KEY_TYPE_POINTER_SIZE;
    return KEY_TYPE_NAME_SIZE;
}

static size_t robin_map_get_entry(const robin_map_t *robin_map, const void *key) {
    size_t key_size = 0;
    if (robin_map->pair_types[0] == 0)
        key_size = strlen(key);
    else key_size = robin_pair_type_size(robin_map->pair_types[0]);

    if (vector_size(robin_map->bucket))
        return siphash(key, key_size, robin_map->secret_key) % vector_size(robin_map->bucket);
    return 0;
}

void * robin_map_get(const robin_map_t * robin_map, const void *key) {
    const size_t index = robin_map_get_entry(robin_map, key);
    robin_map_entry_t * re = vector_get(robin_map->bucket, index);
    if (re)
        return re->valname;
    return nullptr;
}

static void robin_map_fill_field(void * base, const void *input, const uint8_t type) {
    if (!type)
        strcpy(base, input);
    else if (type == 1)
        memcpy(base, input, sizeof(uint64_t));
    else if (type == 2)
        memcpy(base, &input, sizeof(void*));
}
static void robin_map_fill(robin_map_entry_t *re, const void *key, const void *val, uint8_t types[2]) {
    memset(re, 0, sizeof(*re));
    robin_map_fill_field(re->keyname, key, types[0]);
    robin_map_fill_field(re->valname, val, types[1]);

    re->filled = true;
}
static bool robin_map_key_cmp(const robin_map_entry_t *re, const void *key, const uint8_t type) {
    if (!type)
        return strcmp(re->keyname, key) == 0;
    if (type == 1)
        return re->keyval == *(uint64_t*)key;
    if (type == 2)
        return re->keyptr == key;

    return false;
}

void * robin_map_gethash(const robin_map_t *robin_map, const void *key, const uint64_t hash) {
    robin_map_entry_t * re = vector_get(robin_map->bucket, hash % vector_size(robin_map->bucket));
    if (re && robin_map_key_cmp(re, key, robin_map->pair_types[0]))
        return re->keyname;
    return nullptr;
}

bool robin_print(robin_map_entry_t * entry, void *) {
    fprintf(stderr, "psl: %lu, key : %s, value : %s\n", entry->psl_value, entry->keyname, entry->valname);
    return false;
}
void robin_map_print(const robin_map_t *robin_map) {
    robin_map_foreach(robin_map, robin_print, nullptr);
}

void swap_values(void *dest, void *src, const size_t size) {
    uint8_t buffer[size];
    memcpy(buffer, src, size);
    memcpy(src, dest, size);
    memcpy(dest, buffer, size);
}

bool robin_buckets_is_full(const vector_t *buckets) {
    for (size_t i = 0; i < vector_size(buckets); i++) {
        const robin_map_entry_t * re = vector_get(buckets, i);
        if (!re->filled)
            return false;
    }
    return true;
}

bool robin_reorder(const robin_map_entry_t *re, const size_t index, void *user) {
    (void)index;
    robin_map_emplace(user, re->keyname, re->valname);
    return false;
}

bool robin_grow(robin_map_t *robin_map) {
    vector_t *bucket = vector_create(0, sizeof(robin_map_entry_t));
    vector_t *old_bucket = robin_map->bucket;
    vector_setsize(bucket, vector_size(old_bucket) * 2 + 1);

    bool result;
    robin_map->bucket = bucket;

    vector_foreach(old_bucket, robin_reorder, robin_map, result);
    vector_destroy(old_bucket);
    return result;
}

void robin_map_emplace(robin_map_t *robin_map, const void * key, const void * val) {
    robin_map_entry_t entry;
    robin_map_fill(&entry, key, val, robin_map->pair_types);

    size_t *probe = &entry.psl_value;
    if (robin_buckets_is_full(robin_map->bucket))
        robin_grow(robin_map);
    size_t index = robin_map_get_entry(robin_map, entry.keyname);

    while (true) {
        robin_map_entry_t *re = vector_get(robin_map->bucket, index);
        if (!re->filled) {
            *re = entry;
            return;
        }
        if (robin_map_key_cmp(re, entry.keyname, robin_map->pair_types[0]))
            robin_map_fill_field(re->valname, entry.keyname, robin_map->pair_types[1]);

        if (*probe > re->psl_value)
            swap_values(re, &entry, sizeof(*re));
        (*probe)++;

        index++;
        index %= vector_size(robin_map->bucket);
    }
}

void robin_map_foreach(const robin_map_t *robin_map, const robin_callback_t callback, void *user) {
    for (size_t i = 0; i < vector_size(robin_map->bucket); i++) {
        robin_map_entry_t * re = vector_get(robin_map->bucket, i);
        if (re->filled && callback(re, user))
            break;
    }
}

void robin_map_destroy(robin_map_t *robin_map) {
    vector_destroy(robin_map->bucket);
    fb_free(robin_map);
}



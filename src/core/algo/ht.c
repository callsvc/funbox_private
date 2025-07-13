#include <string.h>

#include <types.h>
#include <algo/ht.h>
// https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
uint64_t hash_key_fnv1a(const char *_key) {
    static constexpr uint64_t FNV_prime = 0x00000100000001B3;
    static constexpr uint64_t FNV_offset_basis = 0xCBF29CE484222325;
    uint64_t key = FNV_offset_basis;
    for (const char *k = _key; *k; k++) {
        key ^= (uint8_t)*k;
        key *= FNV_prime;
    }
    return key;
}

ht_t * ht_create(const size_t bucket, const size_t item_size, const char **keys_list) {
    ht_t * ht = fb_malloc(sizeof(ht_t));

    ht->item_size = item_size;

    ht->bucket = vector_create(bucket, sizeof(ht_value_t));
    vector_setsize(ht->bucket, bucket);

    for (size_t i = 0; i < bucket; i++) {
        ht_insert(ht, keys_list[i], nullptr);
    }

    return ht;
}

static ht_value_t * ht_find(const ht_t *ht, const char *key) {
    const uint64_t hash = hash_key_fnv1a(key);
    if (!vector_size(ht->bucket))
        vector_setsize(ht->bucket, 1);
    const uint64_t kindex = hash % vector_size(ht->bucket);

    return vector_get(ht->bucket, kindex);
}

void ht_insert(const ht_t * ht, const char *key, const void *value) {
    ht_value_t * hv = ht_find(ht, key);
    if (strlen(hv->key) && strcmp(key, hv->key)) {
        // The bucket is full, we should expand it and try again
        vector_setsize(ht->bucket, vector_size(ht->bucket) * 2);
        ht_insert(ht, key, value);
        return;
    }
    if (!strlen(hv->key))
        strcpy(hv->key, key);
    if (!hv->value)
        hv->value = fb_malloc(ht->item_size);
    if (value)
        memcpy(hv->value, value, ht->item_size);
}
void * ht_get(const ht_t *ht, const char *key) {
    const ht_value_t * hv = ht_find(ht, key);
    if (!strlen(hv->key) || strcmp(hv->key, key) != 0)
        return nullptr;
    return hv->value;
}

bool ht_contains(const ht_t *ht, const char *key) {
    return ht_get(ht, key) != nullptr;
}

void ht_destroy(ht_t *ht) {
    for (size_t i = 0; i < vector_size(ht->bucket); i++) {
        const ht_value_t * hv = vector_get(ht->bucket, i);
        fb_free(hv->value);
    }
    vector_destroy(ht->bucket);
    fb_free(ht);
}

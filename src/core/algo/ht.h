#pragma once
#include <algo/vector.h>
typedef struct ht_value {
    char key[30];
    void *value;
} ht_value_t;

typedef struct ht {
    vector_t *bucket;
    uint64_t item_size;

} ht_t;

ht_t * ht_create(size_t, size_t, const char **);

void ht_insert(ht_t *, const char*, const void*);
void * ht_get(const ht_t *, const char*);
size_t ht_size(const ht_t*);
void ht_erase(const ht_t*, const char*);
bool ht_contains(const ht_t *, const char*);

void ht_destroy(ht_t*);
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <algo/set.h>

#include <types.h>

set_t * set_create() {
    set_t * set = fb_malloc(sizeof(set_t));
    set->list = list_create(sizeof(setval_t));
    return set;
}

void setval_parser(setval_t *value, const setval_type_e type, const void *data) {
    value->type = type;
    if (type == setval_string)
        strcpy(value->val_string, data);
}
bool setval_eq(const setval_t *value, const setval_type_e type, const void *data) {
    if (type == setval_string)
        return strcmp(value->val_string, data) == 0;
    return false;
}

void set_set(const set_t *set, const setval_type_e k_type, const setval_type_e v_type, const void *first, const void *second) {
    if (set_get(set, k_type, first))
        return;

    setval_t *_key = list_emplace(set->list);
    setval_parser(_key, k_type, first);
    setval_t *value = list_emplace(set->list);
    setval_parser(value, v_type, second);

    value->first = _key;
}
void * set_get(const set_t * set, const setval_type_e type, const void *first) {
    setval_t *value = nullptr;
    for (uint64_t i = 0; i < list_size(set->list) && !value; i++) {
        const setval_t *key = list_get(set->list, i);
        if (key->first || key->type != type)
            continue;
        if (!setval_eq(key, type, first))
            continue;
        value = list_get(set->list, ++i);
    }
    return value;
}

void set_destroy(set_t *set) {
    list_destroy(set->list);
    fb_free(set);
}
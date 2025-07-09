#pragma once
#include <algo/list.h>


typedef enum setval_type {
    setval_undef,
    setval_string,
} setval_type_e;
typedef struct setval {
    union {
        char val_string[100];
    };
    struct setval *first;
    setval_type_e type;
} setval_t;
typedef struct set {
    list_t * list;
} set_t;

set_t * set_create();
void set_set(const set_t *, setval_type_e k_type, setval_type_e, const void *, const void *);
void * set_get(const set_t *, setval_type_e, const void *);

void set_destroy(set_t *);
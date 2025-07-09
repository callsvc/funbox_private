#pragma once

#include <stddef.h>
typedef struct list {
    struct list *next;
    size_t size;
    void *data;
} list_t;

list_t *list_create(size_t);

void * list_emplace(list_t*);
void list_push(list_t *, void *);

size_t list_size(const list_t *);

size_t list_locate(const list_t *, const void *data);
void list_drop(list_t *, size_t);

void * list_get(const list_t*, size_t);
void list_destroy(list_t *);
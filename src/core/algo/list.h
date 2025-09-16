#pragma once

#include <stddef.h>
#include <mimalloc.h>

typedef struct list_node {
    void *data;
    struct list_node * next;
} list_node_t;

typedef struct list {
    size_t size;

    mi_heap_t *heap;
    list_node_t * tail;
    list_node_t *head;
} list_t;

list_t *list_create(size_t);

void * list_emplace(list_t*);
void list_push(list_t *, void *);

size_t list_size(const list_t *);

size_t list_locate(const list_t *, const void *data);
void list_drop(list_t *, size_t);

void * list_get(const list_t*, size_t);
void list_destroy(list_t *);
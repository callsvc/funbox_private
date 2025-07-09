#pragma once
#include <stdbool.h>
#include <stddef.h>

typedef struct vector {
    size_t size;
    size_t type;
    size_t capacity;
    void *data;
} vector_t;

vector_t * vector_create(size_t, size_t);
void * vector_begin(const vector_t*);
size_t vector_resize(vector_t *, size_t);
size_t vector_size(const vector_t*);
void vector_setsize(vector_t*, size_t);
void * vector_get(const vector_t *, size_t);
void * vector_emplace(vector_t *, const void *data);
void vector_destroy(vector_t *);

vector_t * vector_clone(const vector_t *);
bool vector_isequal(const vector_t *, const vector_t *);


#define vector_foreach(vec, callback, user, result)\
    result = false;\
    for (size_t i = 0; i < vector_size(vec); i++) {\
        void * content = vector_get(vec, i);\
        if (callback(content, i, user))\
            if ((result = true))\
                break;\
}


#include <algo/vector.h>

#include <stdlib.h>
#include <string.h>
#include <limits.h>

#include <types.h>

// typesize can be zero to specify a dynamic array of strings
vector_t *vector_create(const size_t count, const size_t typesize) {
    vector_t *vec = fb_malloc(sizeof(vector_t));
    vec->type = typesize;
    if (count)
        vector_resize(vec, count);
    return vec;
}

void * vector_begin(const vector_t *vec) {
    if (!vec->size)
        return nullptr;
    return vec->data;
}

size_t vector_size(const vector_t *vec) {
    if (vec->type)
        return vec->size ? vec->size / vec->type : 0;
    size_t count = 0;
    for (const char *begin = vec->data; strlen(begin) && begin <= (char*)vec->data + vec->size; begin++)
        if ((begin = strchr(begin , '\0')))
            count++;
    return count;
}

void vector_setsize(vector_t *vec, const size_t size) {
    vector_resize(vec, size);
    if (vec->capacity >= size)
        vec->size = size * vec->type;
}

void * vector_get(const vector_t *vec, size_t index) {
    if (vec->type)
        return vec->data + index * vec->type;
    char *result = vec->data;
    while (index--) {
        if ((result = strchr(result, '\0')) == nullptr)
            break;
        result++;
    }
    return result;
}

static void vector_realloc(vector_t *vec, const size_t size) {
    void *result = fb_malloc(size);
    if (!vec->data)
        if ((vec->data = result))
            if ((vec->capacity = size))
                return;
    memcpy(result, vec->data, vec->size);
    fb_free(vec->data);

    vec->capacity = size;
    vec->data = result;
}

size_t vector_resize(vector_t *vec, const size_t count) {
    static const size_t minforstrings = PATH_MAX;
    const size_t resize = vec->type ? count * vec->type : minforstrings;
    if (vec->size < resize)
        vector_realloc(vec, resize);
    else vec->capacity = resize;
    return count;
}

void * vector_emplace(vector_t *vec, const void *data) {
    const size_t realsize = !vec->type ? strlen(data) : vec->type;
    if (!vec->capacity || vec->capacity < vec->size + realsize)
        vector_resize(vec, vec->capacity * 2 + realsize);
    uint8_t *place = &((uint8_t*)vec->data)[vec->size];
    vec->size += realsize != vec->type ? realsize + 1 : vec->type;
    if (data)
        memcpy(place, data, realsize);
    return place;
}

void vector_destroy(vector_t *vec) {
    if (vec->data)
        fb_free(vec->data);
    fb_free(vec);
}

vector_t * vector_clone(const vector_t *clone) {
    vector_t * vec = fb_malloc(sizeof(vector_t));
    vec->size = clone->size;
    vec->type = clone->type;
    vec->capacity = clone->capacity;

    if (!clone->data)
        return vec;

    vec->data = fb_malloc(clone->capacity);
    memcpy(vec->data, clone->data, vec->size);
    return vec;
}

bool vector_isequal(const vector_t * veca, const vector_t * vecb) {
    if (veca->type == vecb->type)
        if (veca->capacity == vecb->capacity)
            if (veca->size == vecb->size)
                if (memcmp(veca->data, vecb->data, veca->size) == 0)
                    return true;
    return false;
}

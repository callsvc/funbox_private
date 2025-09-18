#include <assert.h>
#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <types.h>
#include <algo/ht.h>
#include <mimalloc.h>
ht_t *ht_mm = nullptr;
pthread_mutex_t ht_mutex;

void fb_create();
void fb_destroy();

void fb_exit() {
    fb_destroy();
    pthread_mutex_destroy(&ht_mutex);

    mi_collect(true);
}

__attribute__((constructor)) void fb_entry() {
#if NDEBUG
    mi_option_set(mi_option_verbose, 0);
#endif
    atexit(fb_exit);

    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&ht_mutex, &attr);
    pthread_mutexattr_destroy(&attr);

    fb_create();
}

constexpr size_t malloc_threshold = 2 * 1024 * 1024;
void * fb_malloc(const size_t size) {
    if (!size)
        return nullptr;
    if (unlikely(!ht_mm)) {
        ht_mm = (void*)1; // hack for dealing with dead loops
        if (!pthread_mutex_trylock(&ht_mutex))
            if ((ht_mm = ht_create(0, sizeof(size_t), nullptr)))
                pthread_mutex_unlock(&ht_mutex);
    }

    void *result;
    char buffer[45];
    if (unlikely(size >= malloc_threshold)) {
        pthread_mutex_lock(&ht_mutex);
        result = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        if (result)
            ht_insert(ht_mm, to_str64((uint64_t)result, buffer, 16), &size);
        pthread_mutex_unlock(&ht_mutex);
    } else {
        result = malloc(size);
    }
    memset(result, 0, size);
    return result;
}

void * exchange(void **val, void *eval) {
    void *old = *val;
    *val = eval;
    return old;
}

bool sum_heap(__attribute__((unused)) const mi_heap_t *heap, __attribute__((unused)) const mi_heap_area_t *area, __attribute__((unused)) void *block, const size_t block_size, void *arg) {
    size_t *heap_usage = arg;
    (*heap_usage)++;
    *(heap_usage + 1) += block_size;
    return true;
}
size_t fb_get_heap_usage(size_t *allocs_count) {
    size_t heap_usage[2] = {};
    mi_heap_visit_blocks(mi_heap_get_default(), true, sum_heap, heap_usage);
    if (allocs_count)
        *allocs_count = *heap_usage;
    return *(heap_usage + 1);
}

void fb_free(void *ptr) {

    pthread_mutex_lock(&ht_mutex);
    char buffer[45];
    const size_t size = ht_mm ? (size_t)ht_get(ht_mm, to_str64((uint64_t)ptr, buffer, 16)) : 0;
    if (unlikely(size)) {
        if (munmap(ptr, size))
            quit("ptr %p is invalid", ptr);
        ht_erase(ht_mm, to_str64((uint64_t)ptr, buffer, 16));
    } else if (ptr) {
        assert(mi_check_owned(ptr));
        free(ptr);
    }

    if (ht_mm && !ht_size(ht_mm)) {
        pthread_mutex_unlock(&ht_mutex);
        ht_destroy(exchange((void**)&ht_mm, nullptr));
        pthread_mutex_lock(&ht_mutex);
    }

    pthread_mutex_unlock(&ht_mutex);
}

char * to_binary(const void *value, const size_t size) {
    _Thread_local static char buffer_text[100];
    char * ptr = buffer_text;
    for (const uint8_t * b = value; b < (uint8_t*)value + size; b++) {
        for (uint8_t bi = 0; bi < 8; bi++)
            *ptr++ = *b & 1 << bi ? '1' : '0';
    }
    *(ptr-1) = '\0';
    return buffer_text;
}

const char * to_str64(const uint64_t value, char *buffer, const uint8_t base) {
    switch (base) {
        case 16:
            sprintf(buffer, "%lX", value); break;
        default:
        case 10:
            sprintf(buffer, "%lu", value);
    }
    return buffer;
}
const char * to_str(char *buffer, const uint8_t *bytes, size_t size) {
    char *to = buffer;
    while (size--) {
        constexpr char byte_fmt[] = {"0123456789abcdef"};
        *to++ = byte_fmt[(*bytes >> 4) & 0xF];
        *to++ = byte_fmt[*bytes++ & 0xF];
    }
    *to = '\0';
    return buffer;
}

uint8_t char_to_int(const char val) {
    if (val >= '0' && val <= '9')
        return val - '0';
    if (val >= 'a' && val <= 'f')
        return val - 'a' + 10;
    if (val >= 'A' && val <= 'F')
        return val - 'A' + 10;

    __builtin_unreachable();
}

void * strtobytes(const char *str, void * output, const size_t size) {
    uint8_t *p = output;
    memset(p, 0, size);
    if (strlen(str) > size * 2)
        return nullptr;

    for (const char *p_str = str; *p_str; p_str++) {
        if (unlikely(!(p_str - str)))
            *p = char_to_int(*p_str) << 4;
        else if ((p_str - str) % 2)
            *p++ |= char_to_int(*p_str);
        else *p |= char_to_int(*p_str) << 4;
    }
    return output;
}

#define NS_MAX_VALUE 999999999
void sleep_for(const size_t ns) {
    if (ns < NS_MAX_VALUE) {
        const struct timespec rtime = { 0, (long)ns };
        if (nanosleep(&rtime, nullptr))
            quit("can't sleep");
    } else {
        if (usleep(ns / 1000))
            quit("also can't sleep anyway");
    }
}

uint64_t fb_rand() {
    static _Thread_local uint64_t buffer[0x100];
    static _Thread_local uint64_t bidx;

    if (unlikely(!bidx)) {
        const int32_t rfd = open("/dev/urandom", O_RDONLY);
        bidx = read(rfd, buffer, sizeof(buffer)) / sizeof(uint64_t);
        close(rfd);
    }
    const uint64_t result = buffer[bidx - 1];
    bidx -= 1;
    return result;
}

#pragma once
#include <limits.h>
#include <stdint.h>
#include <time.h>

#include <logger.h>
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

void oskill(const char * format, ...);

extern char username[30];

typedef struct procinfo {
    char current_dir[PATH_MAX];
    time_t start;
} procinfo_t;

char * trim(char *s);
void * fb_malloc(size_t);
void fb_free(void *);
char * fb_strcopy(char *, const char *, size_t);

char* fb_strmk(size_t, ...);

procinfo_t * procinfo_create();
void procinfo_destroy(procinfo_t*);

char * to_binary(const void *, size_t);
const char * to_str64(uint64_t, char *, uint8_t);
void * strtobytes(const char * str, void*, size_t);
uint64_t fb_rand();

char * fb_strdup(const char *str);

#define ms(x) (x * 1000000)

void sleep_for(size_t);
bool cmpsha(const uint8_t *, size_t, const char *);

#define swap_b32(begin) __builtin_bswap32(*(const uint32_t*)(begin))
#define swap_b16(begin) __builtin_bswap16(*(const uint16_t*)(begin))

#define count_of(x) (sizeof(x) / sizeof(*x))

typedef struct span {
    void *data;
    size_t size;
} span_t;


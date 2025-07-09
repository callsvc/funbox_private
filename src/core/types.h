#pragma once
#include <limits.h>
#include <time.h>

#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

void oskill(const char * format, ...);
void envos_issafe();

extern char username[30];

void funbox_create();
void funbox_destroy();

typedef struct procinfo {
    char proc_cwd[PATH_MAX];
    time_t start;
} procinfo_t;

char * trim(char *s);
void * funbox_malloc(size_t);
void funbox_free(void *);
char * funbox_strncpy(char *, const char *, size_t);

char* strings_concat(size_t, ...);

procinfo_t * procinfo_create();
void procinfo_destroy(procinfo_t*);

char * to_binary(const void *, size_t);
void * strtobytes(const char * str, void*, size_t);

#define ms(x) (x * 1000000)

void sleep_for(size_t);

#define big32(begin) __builtin_bswap32(*(const uint32_t*)(begin))
#define big16(begin) __builtin_bswap16(*(const uint16_t*)(begin))

#define count_of(x) (sizeof(x) / sizeof(*x))

typedef struct span {
    void *data;
    size_t size;
} span_t;


#pragma once
#include <limits.h>
#include <stdint.h>
#include <time.h>

#include <logger.h>
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

// grep -E 'MIN\(|MAX\(' /usr/include/sys/param.h
#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

void quit(const char * format, ...);

extern char username[30];

typedef struct sdl_app sdl_app_t;
typedef struct procinfo {
    char current_dir[PATH_MAX];
    char libname[0x30];
    time_t start;

    void *user_ptr;
    sdl_app_t *sdl_toolkit;
} procinfo_t;

char * trim(char *s);
void * fb_malloc(size_t);
void fb_free(void *);
size_t fb_get_heap_usage(size_t *);

char * fb_strcopy(char *, const char *, size_t);

char* fb_strmk(size_t, ...);

procinfo_t * procinfo_create();
void procinfo_destroy(procinfo_t*);

char * to_binary(const void *, size_t);
const char * to_str64(uint64_t, char *, uint8_t);
const char * to_str(char *, const uint8_t*, size_t);
void * strtobytes(const char * str, void*, size_t);
uint64_t fb_rand();

char * fb_strdup(const char *str);

#define ms(x) (x * 1000000)

void sleep_for(size_t);
bool cmpsha(const uint8_t *, size_t, const char *);
typedef struct fsfile fsfile_t;
void fs_sha256(fsfile_t *file, uint8_t out[256 / 8]);

#define to_little64(b) __builtin_bswap64(*(const uint64_t*)(b))
#define to_little32(b) __builtin_bswap32(*(const uint32_t*)(b))
#define to_little16(b) __builtin_bswap16(*(const uint16_t*)(b))

#define count_of(x) (sizeof(x) / sizeof(*x))

typedef struct span {
    void *data;
    size_t size;
} span_t;


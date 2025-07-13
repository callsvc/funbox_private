#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <fcntl.h>
#include <types.h>

void * fb_malloc(const size_t size) {
    if (!size)
        return nullptr;
    void * result = malloc(size);
    memset(result, 0, size);
    return result;
}

void fb_free(void *ptr) {
    if (ptr)
        fb_free(ptr);
}

char * to_binary(const void *value, const size_t size) {
    _Thread_local static char buffer_th[0x100];
    char * ptr = buffer_th;
    for (const uint8_t * b = value; b < (uint8_t*)value + size; b++) {
        for (uint8_t bi = 0; bi < 8; bi++)
            *ptr++ = *b & 1 << bi ? '1' : '0';
        *ptr++ = ' ';
    }
    *(ptr - 1) = '\0';
    return buffer_th;
}

uint8_t char_to_int(const char val) {
    if (val >= '0' && val <= '9')
        return val - '0';
    if (val >= 'a' && val <= 'f')
        return val - 'a' + 10;
    if (val >= 'A' && val < 'F')
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
        const struct timespec rtime = { 0, ns };
        if (nanosleep(&rtime, nullptr))
            oskill("can't sleep");
    } else {
        if (usleep(ns / 1000))
            oskill("also can't sleep anyway");
    }
}

uint64_t fb_rand() {
    static _Thread_local uint64_t buffer[0x100];
    static _Thread_local uint64_t bidx;

    if (unlikely(!bidx)) {
        const int32_t rfd = open("/dev/random", O_RDONLY);
        bidx = read(rfd, buffer, sizeof(buffer)) / sizeof(uint64_t);
        close(rfd);
    }
    const uint64_t result = buffer[bidx - 1];
    bidx -= 1;
    return result;
}

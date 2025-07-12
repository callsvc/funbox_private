#include <ctype.h>
#include <stdarg.h>
#include <string.h>

#include <types.h>

char * strings_concat(const size_t count, ...) {
    size_t size = 0;
    va_list va = {}, cp = {};
    va_start(va, count);
    va_copy(cp, va);
    for (size_t i = 0; i < count; i++)
        size += strlen(va_arg(va, char*));

    char *result = funbox_malloc(size + 1);
    char *ptr = result;
    for (size_t i = 0; i < count; i++) {
        const char *str = va_arg(cp, const char*);
        const size_t len = strlen(str);
        memcpy(ptr, str, len);
        ptr += len;
    }
    va_end(va);
    va_end(cp);
    return result;
}

// https://stackoverflow.com/questions/656542/trim-a-string-in-c
int scape(const char val) {
    return isspace(val) || !isprint(val);
}
char *ltrim(char *s) {
    while(scape(*s)) s++;
    return s;
}

char *rtrim(char *s) {
    char *back = s + strlen(s);
    while(scape(*--back)) {}
    *(back + 1) = '\0';
    return s;
}

char *trim(char *s) {
    return rtrim(ltrim(s)); 
}

char *funbox_strncpy(char * dest, const char * src, const size_t size) {
    strncpy(dest, src, size);
    dest[size] = '\0';
    return dest;
}
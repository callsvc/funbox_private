
#include <stdarg.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

void quit(const char *format, ...) {
    va_list args = {};
    va_start(args, format);

    vfprintf(stderr, format, args);
    va_end(args);

    if (!errno)
        exit(1);

    fprintf(stderr, "\n%s\n", strerror(errno));
    exit(errno);
}

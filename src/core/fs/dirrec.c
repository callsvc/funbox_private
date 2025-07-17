#include <dirent.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include <core/types.h>
#include <fs/types.h>


vector_t * list_all_files(const char *dirpath) {
    DIR* directory = opendir(dirpath);
    if (!directory)
        return nullptr;
    vector_t * vec = vector_create(0, 0);
    for (const struct dirent *entry; (entry = readdir(directory)); )
        if (*entry->d_name != '.')
            vector_emplace(vec, entry->d_name);
    closedir(directory);

    return vec;
}

char * fs_build_path(const int32_t depth, ...) {
    va_list ap = {};
    va_start(ap, depth);

    char * buffer = fb_malloc(PATH_MAX);
    if (!buffer)
        return nullptr;
    *buffer = ':';
    for (int32_t i = 0; i < depth; i++) {
        if (i)
            sprintf(strchr(buffer, ':'), "/:");
        sprintf(strchr(buffer, ':'), "%s:", va_arg(ap, char*));
    }
    *strrchr(buffer, ':') = '\0';
    va_end(ap);
    return buffer;
}

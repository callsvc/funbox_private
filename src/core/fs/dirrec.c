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

const char * fs_get_cache(procinfo_t *info) {
    if (strlen(info->current_dir + 1024))
        return info->current_dir + 1024;
    char *dest = info->current_dir + 1024;

    char cwd_copy[100];
    strncpy(cwd_copy, info->current_dir, sizeof(cwd_copy));

    sprintf(dest, "%s/%s_cache", cwd_copy, info->libname);

    if (!fs_exists(dest))
        quit("cache not found: %s", dest);
    return dest;
}

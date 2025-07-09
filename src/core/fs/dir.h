#pragma once
#include <algo/list.h>
#include <fs/file.h>

typedef struct dir {
    fsdir_t vdir;
    list_t *cached_files;
} dir_t;

dir_t *dir_open(const char*, const char*);
file_t *dir_open_file(const dir_t*, const char*, const char*);
void dir_close_file(const dir_t*, file_t *);

void dir_close(dir_t*);
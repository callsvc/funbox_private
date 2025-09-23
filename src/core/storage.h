#pragma once
#include <algo/list.h>
#include <fs/dir.h>

typedef struct dirreg {
    char name[24];
    dir_t *dir;
} dirreg_t;

typedef struct storage {
    list_t * dirs;

    size_t file_max;
} storage_t;

void storage_init();
void storage_zip(const storage_t*, fsfile_t *);
void storage_opendir(const storage_t*, const char * , const char *);
void storage_destroy();
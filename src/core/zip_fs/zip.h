#pragma once

#include <fs/types.h>
#include <zip.h>

typedef struct zipdir {
    fsdir_t vdir;

    zip_t *zipfile;
} zipdir_t;

typedef struct zipfile {
    fsfile_t vfile;
    size_t size;
    size_t index;

    zip_t *zip;
    zip_file_t *zipentry;
} zipfile_t;

zipdir_t * zipdir_open(const char *);
zipdir_t * zipdir_open_2(file_t *);
void zipdir_close(zipdir_t *);

zipfile_t * zipfile_open(zip_t *, size_t index, size_t);
size_t zipfile_getsize(const zipfile_t*);
void zipfile_close(zipfile_t *);

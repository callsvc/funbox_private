#pragma once
#include <stdio.h>

#include <fs/types.h>

typedef struct dir dir_t;

typedef struct file {
    fsfile_t vfile;
    FILE *handle;
    char *buffer;

    const dir_t *parent;
    vector_t *write_stall;
} file_t;

file_t * file_open(const char*, const char*);

size_t file_getsize(const file_t *file);
void file_write(const file_t *, const void *, size_t, size_t offset);
void file_swrite(const file_t *, const void *, size_t); // stall next write
void file_flush(const file_t*);
void file_read(const file_t*, void*, size_t, size_t);

const char * file_errorpath(const char*);
void file_close(file_t*);
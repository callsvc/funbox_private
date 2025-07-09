#pragma once

#include <stdint.h>
#include <algo/vector.h>


#define MAX_FILEPATH 0x9F

typedef struct fsfile {
    char path[MAX_FILEPATH];
    void (*fs_read)(struct fsfile*, void*, size_t, size_t);
    size_t (*fs_getsize)(const struct fsfile*);

    char buffer[100];
    uint32_t type; // MAPFILE
} fsfile_t;

typedef struct fsdir {
    char path[MAX_FILEPATH];
    char mode[2];
    fsfile_t *(*open_file)(struct fsdir *, const char *, const char *);
    void (*close_file)(struct fsdir*, fsfile_t *);
    vector_t *(*fs_list_all_files)(const struct fsdir*);

} fsdir_t;

fsfile_t * fs_open_file(fsdir_t *, const char *, const char *);
void fs_close_file(fsdir_t *, fsfile_t *);

void fs_read(fsfile_t*, void*, size_t, size_t);
size_t fs_getsize(const fsfile_t*);
vector_t * fs_filebytes(fsfile_t*);

bool fs_is_mapfile(const fsfile_t*);

int32_t fs_exists(const char *);
const char * fs_getpath(const void *);

vector_t * fs_list_all_files(const fsdir_t*);
vector_t * list_all_files(const char*);
char * fs_build_path(int32_t depth, ...);

const char * fs_readline(fsfile_t *file, size_t *offset);

vector_t * fs_grep(const fsdir_t *, const char *);

vector_t * fs_getbytes(fsfile_t *, size_t, uint64_t);
vector_t * fs_getfile(fsfile_t *);
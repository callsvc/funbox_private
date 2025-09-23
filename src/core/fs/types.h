#pragma once

#include <stdint.h>
#include <algo/vector.h>


#define MAX_FILEPATH 0x9F

typedef enum file_type {
    file_type_none,
    file_type_file,
    file_type_mapfile,
    file_type_offsetfile,
    file_type_aes
} file_type_e;

typedef struct fsfile {
    char path[MAX_FILEPATH];
    void (*fs_read)(struct fsfile*, void*, size_t, size_t);
    void (*fs_write)(struct fsfile*, const void*, size_t, size_t);
    size_t (*fs_getsize)(const struct fsfile*);

    char buffer[100];
    file_type_e type;
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
void fs_write(fsfile_t*, const void*, size_t, size_t);

void fs_mkdir(const char *, bool);
void touch(const char*);

size_t fs_getsize(const fsfile_t*);
vector_t * fs_filebytes(fsfile_t*);

bool fs_is_mapfile(const fsfile_t*);

int32_t fs_exists(const char *);
bool fs_exists_in_fsdir(const vector_t *, const char*);
const char * fs_getpath(const void *);
void fs_print_tree(const vector_t *);

vector_t * fs_list_all_files(const fsdir_t*);
vector_t * list_all_files(const char*);
char * fs_build_path(int32_t depth, ...);

bool fs_rm(const char *path);
bool fs_isro(const char *mode);

typedef struct procinfo procinfo_t;
const char * fs_get_cache(procinfo_t*);

const char * fs_readline(fsfile_t *file, size_t *offset);

vector_t * fs_grep(const fsdir_t *, const char *);

vector_t * fs_getbytes(fsfile_t *, size_t, uint64_t);
vector_t * fs_getfile(fsfile_t *);
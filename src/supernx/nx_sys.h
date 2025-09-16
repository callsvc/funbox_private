#pragma once
#include <types.h>
#include <loader/types.h>
#include <horizon/hos.h>

typedef struct game_file {
    loader_type_e type;
    fsfile_t * file;
    loader_base_t *loader;

} game_file_t;

typedef struct nx_sys {
    procinfo_t *procinfo;
    hos_t *hos;

    loader_base_t *loader;

    list_t *games;
} nx_sys_t;

nx_sys_t *nx_sys_create();

void nx_get_all_loaders(const nx_sys_t *);
size_t nx_get_games_count(const nx_sys_t*);
void nx_load_first_one(nx_sys_t *nx);

loader_base_t * nx_getgame(const nx_sys_t *, size_t);

void nx_sys_destroy(nx_sys_t *);
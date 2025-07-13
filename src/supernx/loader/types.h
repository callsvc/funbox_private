#pragma once

#include <fs/types.h>

typedef enum loader_type {
    loader_unknown_type,
    loader_nsp_type,
} loader_type_e;


typedef struct loader_base loader_base_t;
typedef struct loader_base {
    vector_t *(*loader_get_logo)(loader_base_t*);
    uint64_t (*loader_get_program_id)(loader_base_t*);
} loader_base_t;

vector_t * loader_get_logo(loader_base_t *);
uint64_t loader_get_program_id(loader_base_t*);

loader_type_e loader_get_extension_type(const fsfile_t *);
loader_type_e loader_get_rom_type(fsfile_t *);
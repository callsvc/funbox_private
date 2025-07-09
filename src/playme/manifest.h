#pragma once
#include <algo/set.h>

#include <fs/types.h>

typedef struct manifest {
    set_t *attrs;
} manifest_t;

manifest_t * manifest_create(fsfile_t *file);

const char * manifest_get_str(const manifest_t *, const char*);
void manifest_destroy(manifest_t*);
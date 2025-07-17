#pragma once
#include <types.h>

#include <algo/vector.h>
typedef struct config {
    procinfo_t *procinfo;
    const char *package_filepath;

    vector_t *ipa_array;
} config_t;

config_t * config_create();
void config_destroy(config_t*);
#pragma once
#include <types.h>

typedef struct config {
    procinfo_t *procinfo;
    const char *package_filepath; // !< Path to the loaded application's package or directory
} config_t;

config_t * config_create();
void config_destroy(config_t*);
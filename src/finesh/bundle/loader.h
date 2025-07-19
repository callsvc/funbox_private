#pragma once

#include <fsh/plist.h>

typedef struct loader {
    fsdir_t *ipa_pkg;
    vector_t *ipa_files;

    plist_t *info_plist;

} loader_t;

loader_t * loader_create(const char *path);
void loader_destroy(loader_t*);
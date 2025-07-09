#pragma once
#include <horizon/keys_db.h>

typedef struct hos {
    keys_db_t *kdb;
} hos_t;

hos_t *hos_create(const char*);

void hos_destroy(hos_t *);

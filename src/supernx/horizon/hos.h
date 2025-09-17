#pragma once
#include <loader/types.h>
#include <horizon/keys_db.h>

#include <hle_services/types.h>
typedef struct hos {
    keys_db_t *kdb;
    list_t *services;
} hos_t;

hos_t *hos_create(const char*);

void hos_destroy(hos_t *);

void hos_enable(const char *, hos_t*, loader_base_t*); // enable everything, services, processes, the kernel and others stuffs
void hos_disable(hos_t*);


size_t hos_getprocess_count(hos_t*);
void * hos_continue(hos_t*);

bool hos_is_titleicon(fsfile_t *);
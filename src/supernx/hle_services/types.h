#pragma once

typedef struct hle_service {
    char service_name[64];
} hle_service_t;


typedef struct content_archive content_archive_t;
typedef struct hos hos_t;
typedef void (*service_load_func_t)(content_archive_t*, hos_t*);

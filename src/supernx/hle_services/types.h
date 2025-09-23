#pragma once

typedef enum service_type {
    service_type_unknown,
    service_type_system_version_title
} service_type_e;

typedef struct hle_service {
    service_type_e type;
    char service_name[64];
} hle_service_t;


typedef struct content_archive content_archive_t;
typedef struct hos hos_t;
typedef void (*service_load_func_t)(const content_archive_t*, const hos_t*);

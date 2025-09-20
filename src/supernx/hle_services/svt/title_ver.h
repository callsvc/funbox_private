#pragma once

#include <stdint.h>
#include <hle_services/types.h>

struct svt_format {
    uint8_t major;
    uint8_t minor;
    uint8_t micro;
    uint8_t pad0;

    uint8_t rev_major;
    uint8_t rev_minor;
    uint16_t pad1;

    char platform[0x20];
    char version_hash[0x40]; // 0x28-bytes(not including NUL-terminator) normally with zeros afterward
    char display_version[0x18];
    char display_title[0x80];
};

typedef struct svt_service {
    hle_service_t service_base;
    struct svt_format content;
} svt_service_t;

void svt_load(const content_archive_t*, const hos_t*);
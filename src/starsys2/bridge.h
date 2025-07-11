#pragma once
#include <stdint.h>

typedef struct sony sony_t;

typedef struct bridge {
    sony_t * pstwo;
    uint8_t *ee_memory;
} bridge_t;

bridge_t *bridge_create(sony_t *);
void bridge_destroy(bridge_t *);
uint32_t bridge_read(const bridge_t*, uint32_t);
void bridge_write(const bridge_t*, uint32_t, uint32_t);
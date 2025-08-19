#pragma once

#include <stdint.h>

typedef struct cache_line {
    uint32_t tags;
    uint32_t inst[4];
} cache_line_t;

typedef struct cache {
    cache_line_t lines[256];
} cache_t;

void cache_reset(cache_t *);
void cache_set(cache_t *, uint32_t, uint32_t, uint32_t[4]);
void cache_set2(cache_t *, uint32_t, uint32_t[4]);

int32_t cache_hit(const cache_t *cache, uint32_t addr, bool *hit);
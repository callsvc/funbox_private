
#include <string.h>

#include <cache.h>
void cache_reset(cache_t *cache) {
    memset(cache->lines, 0, sizeof(cache->lines));
}

void cache_set(cache_t *cache, const uint32_t addr, const uint32_t index, uint32_t inst[4]) {
    const uint32_t line_addr = (addr >> 4) & 0xFF;
    cache_line_t *line = &cache->lines[line_addr];
    for (size_t i = index; i < 4; i++)
        line->inst[i] = inst[i];

    line->tags = ((addr >> 12) << 12) | 1;
}
void cache_set2(cache_t *cache, const uint32_t addr, uint32_t values[4]) {
    for (size_t i = 0; i < 4; i++)
        cache->lines[addr >> 4].inst[i] = values[i];

    cache->lines[addr >> 4].tags = 0;
}


int32_t cache_hit(const cache_t *cache, const uint32_t addr, bool *hit) {
    *hit = 0;

    const cache_line_t *line = &cache->lines[(addr >> 4) & 0xFF];
    if (line->tags & 1 && (addr & 0xFFFFF000) == (line->tags & 0xFFFFF000))
        *hit = true;

    return (*hit) ? line->inst[(addr >> 2) & 3] : 0;
}

#include <stdio.h>
#include <string.h>

#include <vm/cpool.h>
#include <vm/types.h>

#include <types.h>

uint16_t read_pvalue16(const uint8_t **begin) {
    const uint16_t value = swap_b16(*begin);
    *begin += sizeof(value);
    return value;
}
uint32_t read_pvalue32(const uint8_t **begin) {
    const uint32_t value = swap_b32(*begin);
    *begin += sizeof(value);
    return value;
}
void read_utf8str(const uint8_t **begin, char *out) {
    const uint16_t len = read_pvalue16(begin);
    fb_strcopy(out, (char*)*begin, len);
    *begin += len;
}

cpool_t *cpool_create(const uint8_t *begin, const size_t size) {
    cpool_t *pool = fb_malloc(sizeof(cpool_t));
    pool->constants = list_create(sizeof(cp_entry_t));

    const uint8_t *bytes = begin;
    const uint16_t count = read_16(&bytes);

    if (size < count * 4)
        return nullptr;

    for (uint16_t i = 1; i < count; i++) {
        cp_entry_t *entry = list_emplace(pool->constants);
        entry->type = (constant_pool_type_e)*bytes++;

        switch (entry->type) {
            case cp_type_utf8:
                read_utf8str(&bytes, entry->utf8);
                break;
            case cp_type_integer:
                entry->integer = (int32_t)read_pvalue32(&bytes);
                break;
            case cp_type_string:
            case cp_type_class:
                entry->pair_16[0] = read_pvalue16(&bytes);
                break;
            case cp_type_fieldref:
            case cp_type_methodref:
            case cp_type_name_and_type:
                entry->pair_16[0] = read_pvalue16(&bytes);
                entry->pair_16[1] = read_pvalue16(&bytes);
                break;
            default:
                oskill("not implemented (%u)", entry->type);
        }
    }
    pool->size = bytes - begin;
    return pool;
}

const char * cpool_utf8(const cpool_t * pool, const size_t index) {
    const cp_entry_t *entry = list_get(pool->constants, index - 1);
    if (entry && entry->type == cp_type_utf8)
        return entry->utf8;
    return nullptr;
}
cp_entry_t * cpool_get(const cpool_t *pool, const size_t index) {
    if (index >= list_size(pool->constants))
        return nullptr;
    return list_get(pool->constants, index - 1);
}

void cpool_strings(const cpool_t *pool, FILE *file) {
    for (size_t index = 0; index < list_size(pool->constants); index++) {
        const cp_entry_t *value = list_get(pool->constants, index);
        if (value->type == cp_type_utf8)
            fprintf(file, "%s ", value->utf8);
    }
    fprintf(file, "\n");
}

void cpool_destroy(cpool_t *pool) {
    list_destroy(pool->constants);
    fb_free(pool);
}
#include <vm/types.h>
#include <types.h>
uint16_t read_16(const uint8_t **src) {
    const uint16_t result = big16(*src);
    *src += sizeof(result);
    return result;
}
uint32_t read_32(const uint8_t **src) {
    const uint32_t result = big32(*src);
    *src += sizeof(result);
    return result;
}

void read_attr_values(type_attr_t *values, const uint8_t **src) {
    values->access_flags = read_16(src);
    values->name_index = read_16(src);
    values->descriptor_index = read_16(src);
    values->attrs_count = read_16(src);
}
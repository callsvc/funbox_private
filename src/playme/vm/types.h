#pragma once
#include <stdint.h>

#include <vm/cpool.h>
typedef struct types {
    cpool_t *contants;
    const char *name;

    uint32_t version;

    uint16_t access_flags, this_class, super_class;

    uint16_t interfaces_count;
    uint16_t fields_count;
    uint16_t methods_count;

    list_t *methods;

} class_t;

typedef struct type_attr {
    uint16_t access_flags;
    uint16_t name_index;
    uint16_t descriptor_index;
    uint16_t attrs_count;
} type_attr_t;

uint16_t read_16(const uint8_t **src);
uint32_t read_32(const uint8_t **src);
void read_attr_values(type_attr_t *values, const uint8_t **src);

typedef struct exception_table {
} exception_table_t;

typedef struct method_frame {
    uint32_t pc;
    uint32_t sp;

    void **operand_stack;
    void **locals;
} method_frame_t;

typedef struct method {
    class_t *parent; // < Class that this method belongs to
    type_attr_t attr;

    size_t size;
    method_frame_t method_frame;

    const char *name;
    const char *descriptor;

    uint16_t max_locals;
    uint16_t max_stack;
    uint8_t *bytecode;
    uint8_t *bytecode_end;
} method_t;

class_t * class_create(const uint8_t*, size_t);
void class_destroy(class_t *);

method_t * method_create(class_t *, const uint8_t*, size_t);
void method_destroy(method_t*);

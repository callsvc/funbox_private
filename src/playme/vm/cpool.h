#pragma once
#include <stdio.h>
#include <stdint.h>

#include <algo/list.h>

typedef enum constant_pool_type {
    cp_type_utf8 = 1,
    cp_type_integer = 3,
    cp_type_float,
    cp_type_long,
    cp_type_double,
    cp_type_class,
    cp_type_string,
    cp_type_fieldref,
    cp_type_methodref,
    cp_type_name_and_type = 12
} constant_pool_type_e;


typedef struct cp_fieldref {
    uint16_t class_id;
    uint16_t name_type_id;
} cp_fieldref_t;
typedef struct cp_string {
    uint16_t string_id;
} cp_string_t;

typedef cp_fieldref_t cp_methodref_t;

typedef struct cp_nameandtype {
    uint16_t name_id;
    uint16_t descriptor_id;
} cp_nameandtype_t;

typedef uint16_t cp_class_t;

typedef struct cp_entry {
    constant_pool_type_e type;
    union {
        char utf8[0x100]; // first 2 bytes is length
        int32_t integer;
        uint16_t pair_16[2];
        cp_fieldref_t fieldref;
        cp_methodref_t methodref;
        cp_nameandtype_t nameandtype;
        cp_class_t class;
        cp_string_t string;
    };
} cp_entry_t;

typedef struct cpool {
    list_t *constants;
    size_t size; // !< the Constant Pool size in memory
} cpool_t;

cpool_t *cpool_create(const uint8_t *, size_t);

const char * cpool_utf8(const cpool_t *, size_t);
cp_entry_t * cpool_get(const cpool_t *, size_t);
void cpool_strings(const cpool_t *, FILE *);
void cpool_destroy(cpool_t *);
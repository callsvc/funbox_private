#pragma once

#include <fs/types.h>
#include <algo/list.h>

typedef struct plist_dict {
    const char *key;
    list_t * values;
} plist_dict_t;

typedef enum plist_value_type {
    plist_value_none,
    plist_value_dict,
    plist_value_array,
    plist_value_string,
    plist_value_int,
    plist_value_double,
    plist_value_datetime,
    plist_value_bool,
    plist_value_data
} plist_value_type_e;

typedef struct plist_value plist_value_t;
typedef struct plist_value {
    plist_value_type_e type;
    list_t * list; // list of plist_value_t values, for dict and array
    union {
        char string[100];
        int64_t int_value;
        union {
            double double_value;
            double date_value;
        };
        bool bool_value;
        vector_t * data_value;
    };

    plist_value_t * parent;
} plist_value_t;

typedef struct plist {
    plist_value_t *root;
} plist_t;

plist_t * plist_create(fsfile_t *);
const char * plist_getstr(const plist_t *, const char *);
void plist_destroy(plist_t *);
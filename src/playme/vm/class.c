#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <vm/types.h>

#include <types.h>
const char* class_getversion(const uint8_t *src, const size_t len) {
    if (len < 8)
        return 0;
    if (big32(src) != 0xCAFEBABE)
        return 0;
    if (big16(src + 6) == 0x2D)
        return "JVM 1.0";
    return NULL;
}
uint32_t class_tover(const char *version) {
    if (version && strcmp(version, "JVM 1.0") == 0)
        return 1;
    return 0;
}

void class_print(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
}


void class_setname(class_t * class) {
    const cp_entry_t *clazz = cpool_get(class->contants, class->this_class);
    assert(clazz->type == cp_type_class);

    class->name = cpool_utf8(class->contants, clazz->class);
}

class_t * class_create(const uint8_t *begin, const size_t size) {
    class_t *class = funbox_malloc(sizeof(class_t));

    uint32_t version = 0;
    if ((version = class_tover(class_getversion(begin, size))) == 0)
        return NULL;
    class->contants = cpool_create(begin + 8, size - 8);
    class->version = version;

#if 1
    cpool_strings(class->contants, stderr);
#endif

    const uint8_t *class_info = begin + class->contants->size + 8;
    class->access_flags = read_16(&class_info);
    class->this_class = read_16(&class_info);

    class_setname(class);

    class->super_class = read_16(&class_info);

    class->interfaces_count = read_16(&class_info);
    for (size_t i = 0; i < class->interfaces_count; i++)
        read_16(&class_info);

    class->fields_count = read_16(&class_info);
    for (size_t i = 0; i < class->fields_count; ++i) {
        type_attr_t attr;
        read_attr_values(&attr, &class_info);
        for (size_t j = 0; j < attr.attrs_count; ++j) {
            const uint32_t len = read_32(&class_info);
            class_info += len;
        }
    }

    class->methods_count = read_16(&class_info);
    class->methods = list_create(0);
    for (size_t i = 0; i < class->methods_count; i++) {
        method_t *method = method_create(class, class_info, class_info - begin);
        list_push(class->methods, method);
        class_info += method->size;
    }

    return class;
}
void class_destroy(class_t *class) {
    if (class->methods)
        for (size_t i = 0; i < list_size(class->methods); i++)
            method_destroy(list_get(class->methods, i));
    list_destroy(class->methods);
    if (class->contants)
        cpool_destroy(class->contants);

    free(class);
}
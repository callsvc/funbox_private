#include <stdlib.h>
#include <string.h>
#include <vm/types.h>

#include <types.h>

void method_setup_frame(method_t *method) {
    method_frame_t *frame = &method->method_frame;

    frame->locals = funbox_malloc(sizeof(void*) * (method->max_locals + 1));
    frame->operand_stack = funbox_malloc(sizeof(void*) * (method->max_stack + 1));
}

void method_setcode(method_t *method, const uint8_t **source, const size_t size) {
    method->max_stack = read_16(source);
    method->max_locals = read_16(source);

    const uint32_t length = read_32(source);
    if (length > size)
        return;

    method->bytecode = funbox_malloc(length);
    memcpy(method->bytecode, *source, length);
    method->bytecode_end = method->bytecode + length;

    method_setup_frame(method);

    *source += size - 8;
}

method_t * method_create(class_t *class, const uint8_t *begin, const size_t size) {
    method_t * method = funbox_malloc(sizeof(method_t));
    if (size < 8)
        return nullptr;

    const uint8_t * method_info = begin;
    method->parent = class;
    read_attr_values(&method->attr, &method_info);

    method->name = cpool_utf8(method->parent->contants, method->attr.name_index);
    method->descriptor = cpool_utf8(method->parent->contants, method->attr.descriptor_index);

    for (size_t i = 0; i < method->attr.attrs_count; i++) {
        const uint16_t name = read_16(&method_info);
        if (strcmp(cpool_utf8(class->contants, name), "Code") == 0) {
            const uint32_t length = read_32(&method_info);
            method_setcode(method, &method_info, length);
        }
    }

    method->size = method_info - begin;
    return method;
}
void method_destroy(method_t *method) {
    if (method->bytecode)
        funbox_free(method->bytecode);

    const method_frame_t *frame = &method->method_frame;
    if (frame->locals)
        free(frame->locals);
    if (frame->operand_stack)
        free(frame->operand_stack);
    funbox_free(method);
}
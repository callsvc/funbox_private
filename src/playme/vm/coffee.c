
#include <string.h>
#include <types.h>
#include <vm/coffee.h>
#include <vm/types.h>
#define COFFEE_STACK_SIZE (16 * 1024)
coffee_t * coffee_vm() {
    coffee_t * vm = fb_malloc(sizeof(coffee_t));
    vm->pc = vm->sp = COFFEE_STACK_SIZE;
    vm->stack = fb_malloc(COFFEE_STACK_SIZE);

    vm->classes = list_create(0);
    return vm;
}

method_t * coffee_find_method(const coffee_t *vm, const char * name) {
    for (size_t i = 0; i < list_size(vm->classes); i++) {
        const class_t * class = list_get(vm->classes, i);
        for (size_t j = 0; j < list_size(class->methods); j++) {
            method_t * method = list_get(class->methods, j);
            if (strcmp(method->name, name) == 0)
                return method;
        }
    }
    return nullptr;
}

void coffee_run(coffee_t *vm, const vector_t *all_classes) {
    for (size_t i = 0; i < vector_size(all_classes); i++) {
        const span_t *class = vector_get(all_classes, i);
        list_push(vm->classes, class_create(class->data, class->size));
    }
    // Searching for the main program's `<init>` method
    method_t * init = coffee_find_method(vm, "<init>");
    const uint8_t *code = init->bytecode;

    for (size_t i = 0; i < init->bytecode_end - init->bytecode; i++)
        fprintf(stderr, "%x ", code[i]);
    fprintf(stderr, "\n");

    vm->intint = true;
    interpreter(vm, init);
}

void coffee_reset_state(coffee_t *vm) {
    vm->pc = 0;
    vm->sp = COFFEE_STACK_SIZE;
}

inline void coffee_destroy(coffee_t *vm) {
    for (size_t i = 0; i < list_size(vm->classes); i++)
        class_destroy(list_get(vm->classes, i));
    list_destroy(vm->classes);
    fb_free(vm->stack);
    fb_free(vm);
}

#pragma once
#include <stdbool.h>
#include <stdint.h>

#include <algo/vector.h>
#include <algo/list.h>

#include <vm/types.h>

typedef struct {
    uint32_t pc;
    uint32_t sp;
    void *stack;

    list_t *classes; // < All J2ME loaded classes

    bool intint;
} coffee_t;

// Puts the virtual machine into a known state
coffee_t * coffee_vm();
void coffee_run(coffee_t *, const vector_t*);

void coffee_reset_state(coffee_t *);
void coffee_destroy(coffee_t *);

method_t * coffee_find_method(const coffee_t *, const char *);

void interpreter(coffee_t *, method_t*);

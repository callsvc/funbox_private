#include <assert.h>
#include <core/types.h>

#include <vm/cpool.h>
#include <vm/coffee.h>

void invokevirtual(coffee_t *vm, const cpool_t *pool, const uint8_t *pc) {
    const uint16_t methodref_index = swap_b16((uint16_t*)pc);
    const cp_entry_t *refmethod = cpool_get(pool, methodref_index);
    assert(refmethod->type == cp_type_methodref);

    const cp_entry_t *class = cpool_get(pool, refmethod->methodref.class_id);
    const cp_entry_t *name_and_type = cpool_get(pool, refmethod->methodref.name_type_id);

    const char *virt_method = cpool_utf8(pool, name_and_type->nameandtype.name_id);
    fprintf(stderr, "invoking static method %s from %s\n", virt_method, cpool_utf8(pool, *class->pair_16));

    vm->intint = false;
}

void interpreter(coffee_t *vm, method_t *method) {
    const uint8_t *code = method->bytecode;
    const size_t code_len = method->bytecode_end - method->bytecode;

    method_frame_t *frame = &method->method_frame;
    if (!frame->sp && !frame->pc) {
        if (!frame->locals || !frame->operand_stack)
            return;
    }

    const cpool_t *pool = method->parent->contants;
    fprintf(stderr, "inside method: %s\n", method->name);

    while (vm->intint) {
        if (frame->pc > code_len)
            break;
        const uint8_t opcode = code[frame->pc++];
        switch (opcode) {
            case 0x19: {
                const uint8_t index = code[frame->pc++];
                frame->operand_stack[++frame->sp] = frame->locals[index];
                break;
            }
            case 0x2A:
            case 0x2B:
            case 0x2C:
            case 0x2D:
                frame->operand_stack[++frame->sp] = frame->locals[code[frame->pc - 1] - 0x2A];
                break;
            case 0xB7:
                invokevirtual(vm, pool, &code[frame->pc]);
                frame->pc += 2;
                break;
            default:
                oskill("invalid opcode %u", opcode);
        }
    }
}

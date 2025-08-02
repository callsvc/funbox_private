#include <stdlib.h>
#include <types.h>
#include <algo/ht.h>
#include <arm/ir/types.h>

#include <arm/frontend/aarch64/arm64_types.h>

void nop2ir(list_t *ir_list) {
    const ir_t nop = {
        .type = ir_instruction_nop,
    };
    *(ir_t*)list_emplace(ir_list) = nop;
}

#define NOP_TABLE_INDEX 0
arm64_instruction_t static_table_64[];


static ht_t * table = nullptr;
_Thread_local char buffer[30];
void arm64_build_hashtable() {
    ht_insert(table, to_str64(NOP2_IR_OPCODE, buffer, 16), &static_table_64[NOP_TABLE_INDEX]);
}
void arm64_destroy() {
    ht_destroy(table);
}

void arm64_lookup_table(arm64_instruction_t *ids, const uint32_t inst) {
    if (!table) {
        table = ht_create(0, sizeof(arm64_instruction_t), nullptr);
        arm64_build_hashtable();
        atexit(arm64_destroy);
    }

    if (!ht_contains(table, to_str64(inst, buffer, 16)))
        return;
    *ids = *(arm64_instruction_t*)ht_get(table, to_str64(inst, buffer, 16));
}

arm64_instruction_t static_table_64[] = {
    {"nop", nop2ir}
};

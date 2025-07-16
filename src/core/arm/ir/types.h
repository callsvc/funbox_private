#pragma once

typedef enum ir_instruction_type {
    ir_instruction_nop
} ir_instruction_type_e;

typedef struct ir {
    ir_instruction_type_e type;
} ir_t;
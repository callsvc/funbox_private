#pragma once
#include <types.h>

typedef uint16_t x86_flags_t;

#define X86_FLAGS_BIT_INT 9
#define X86_FLAGS_BIT_DIRECTION 10
void x86_flags_set(x86_flags_t*, uint8_t, uint8_t);

typedef struct dospc dospc_t;
typedef struct pc_cpu {
    uint16_t ip; // also known as pc in risc arch
    uint16_t cs;

    x86_flags_t flags;
    dospc_t *pc;

    bool int_;
} pc_cpu_t;

pc_cpu_t* pc_cpu_create();
void pc_cpu_reset(pc_cpu_t *);
void pc_cpu_run(pc_cpu_t *);
void pc_cpu_destroy(pc_cpu_t *);
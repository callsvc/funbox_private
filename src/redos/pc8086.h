#pragma once
#include <types.h>

typedef uint16_t x86_flags_t;

#define X86_FLAGS_INT 9
#define X86_FLAGS_DIRECTION 10
#define X86_FLAGS_ZERO 6

typedef struct dospc dospc_t;
typedef struct pc_8086 {
    uint16_t ip; // also known as pc in risc arch
    uint16_t es;
    uint16_t cs;
    uint16_t ss;
    uint16_t ds;

    uint16_t bp; // base pointer
    uint16_t sp;

    uint16_t si, di;
    union {
        struct {
            uint16_t ax, bx, cx, dx;
        };
        uint16_t regs86[4];
    };

    x86_flags_t flags;
    dospc_t *pc;

    bool int_;
} pc8086_t;

void x86_flags_set(pc8086_t*, uint8_t, uint8_t);
int32_t x86_flags_get(const pc8086_t*, uint8_t);


pc8086_t* pc8086_create();
void pc8086_reset(pc8086_t *);
void pc8086_run(pc8086_t *);
void pc8086_destroy(pc8086_t *);
#pragma once

#include <arm/dynrec.h>

void jit_run(const dynrec_core_t *);
void jit_set_memory(const dynrec_core_t *, uint8_t *, size_t);

jit_cfg_block_t * jit_compile(const dynrec_core_t *, uint64_t start_pc);



uint8_t jit_read8(const dynrec_core_t *, uint64_t);
uint16_t jit_read16(const dynrec_core_t *, uint64_t);
uint32_t jit_read32(const dynrec_core_t *, uint64_t);
uint64_t jit_read64(const dynrec_core_t *, uint64_t);
#pragma once

#include <arm/dynrec.h>

void jit_run(dynrec_core_t *core);

jit_cfg_block_t * jit_compile(dynrec_core_t *core, uint64_t start_pc);
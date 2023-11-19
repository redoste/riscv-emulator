#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>

#include "isa.h"

typedef struct cpu_t {
	guest_paddr pc;
	guest_reg regs[REG_COUNT];
	bool jump_pending;
} cpu_t;

// NOTE : forward declaration to deal with cyclic dependency with emulator.h
typedef struct emulator_t emulator_t;

bool cpu_decode(uint32_t encoded_instruction, ins_t* decoded_instruction);

void cpu_execute(emulator_t* emu);

#endif

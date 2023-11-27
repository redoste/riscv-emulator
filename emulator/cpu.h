#ifndef CPU_H
#define CPU_H

#include <stdbool.h>
#include <stdint.h>

#include "isa.h"

/* cpu_t : structure storing the current state of the emulated CPU
 */
typedef struct cpu_t {
	guest_paddr pc;
	guest_reg regs[REG_COUNT];

	cached_ins_t* instruction_cache;
	guest_paddr instruction_cache_mask;

	bool jump_pending;
} cpu_t;

// NOTE : forward declaration to deal with a cyclic dependency with emulator.h
typedef struct emulator_t emulator_t;

/* cpu_decode : decode a single encoded instruction to a ins_t struct
 *              returns true if the instruction was successfully decoded
 *              returns false otherwise
 *     uint32_t encoded_instruction : encoded instruction in host endianness
 *     ins_t* decoded_instruction   : pointer to the ins_t to fill with the decoded instruction
 */
bool cpu_decode(uint32_t encoded_instruction, ins_t* decoded_instruction);

/* cpu_decode_and_cache : try to get an instruction in the instruction cache, decode it
 *                        and fill the cache in case of a cache miss
 *                        returns true if the instruction was successfully decoded
 *                        returns false otherwise
 *     emulator_t* emu              : pointer to the emulator where the cache will be updated
 *     guest_paddr instruction_addr : address of the instruction to decode and cache
 *     ins_t** decoded_instruction  : pointer to the ins_t* to fill with the address of the decoded instruction
 */
bool cpu_decode_and_cache(emulator_t* emu, guest_paddr instruction_addr, ins_t** decoded_instruction);

/* cpu_invalidate_instruction_cache : invalidate an entry in the instruction cache
 *     emulator_t* emu  : pointer to the emulator where the cache will be updated
 *     guest_paddr addr : address of the instruction to invalidate
 */
void cpu_invalidate_instruction_cache(emulator_t* emu, guest_paddr addr);

/* cpu_execute : execute a single instruction at the CPU PC
 *     emulator_t* emu : pointer to the emulator state to update to the next instruction
 */
void cpu_execute(emulator_t* emu);

#endif

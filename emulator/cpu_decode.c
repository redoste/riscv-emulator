#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/mman.h>

#include "cpu.h"
#include "dynarec_x86_64.h"
#include "emulator.h"
#include "isa.h"

bool cpu_decode(uint32_t encoded_instruction, ins_t* decoded_instruction) {
	uint8_t opcode = DECODE_GET_OPCODE(encoded_instruction);
	// RISC-V I opcodes always have their lower two bits set
	if ((opcode & 3) != 3) {
		return false;
	}
	decoded_instruction->type = INS_TYPES[opcode >> 2];

	switch (decoded_instruction->type) {
		case INS_TYPE_R: {
			decoded_instruction->rd = DECODE_GET_RD(encoded_instruction);
			decoded_instruction->rs1 = DECODE_GET_RS1(encoded_instruction);
			decoded_instruction->rs2 = DECODE_GET_RS2(encoded_instruction);

			uint8_t funct3 = DECODE_GET_F3(encoded_instruction);
			uint8_t funct7 = DECODE_GET_F7(encoded_instruction);

			/* We drop the `aq` and `rl` bits in AMO instructions to simplify the instruction
			 * dispatching as they won't be used anyway
			 */
			if (opcode == OPCODE_AMO) {
				funct7 &= ~3;
			}

			decoded_instruction->opcode_switch = (opcode >> 2) | (funct3 << 5) | (funct7 << 8);
			break;
		}
		case INS_TYPE_I: {
			decoded_instruction->rd = DECODE_GET_RD(encoded_instruction);
			decoded_instruction->rs1 = DECODE_GET_RS1(encoded_instruction);
			decoded_instruction->imm = DECODE_GET_I_IMM(encoded_instruction);

			uint8_t funct3 = DECODE_GET_F3(encoded_instruction);
			decoded_instruction->opcode_switch = (opcode >> 2) | (funct3 << 5);
			break;
		}
		case INS_TYPE_S: {
			decoded_instruction->rs1 = DECODE_GET_RS1(encoded_instruction);
			decoded_instruction->rs2 = DECODE_GET_RS2(encoded_instruction);
			decoded_instruction->imm = DECODE_GET_S_IMM(encoded_instruction);

			uint8_t funct3 = DECODE_GET_F3(encoded_instruction);
			decoded_instruction->opcode_switch = (opcode >> 2) | (funct3 << 5);
			break;
		}
		case INS_TYPE_B: {
			decoded_instruction->rs1 = DECODE_GET_RS1(encoded_instruction);
			decoded_instruction->rs2 = DECODE_GET_RS2(encoded_instruction);
			decoded_instruction->imm = DECODE_GET_B_IMM(encoded_instruction);

			uint8_t funct3 = DECODE_GET_F3(encoded_instruction);
			decoded_instruction->opcode_switch = (opcode >> 2) | (funct3 << 5);
			break;
		}
		case INS_TYPE_U:
			decoded_instruction->rd = DECODE_GET_RD(encoded_instruction);
			decoded_instruction->imm = DECODE_GET_U_IMM(encoded_instruction);
			decoded_instruction->opcode_switch = opcode >> 2;
			break;
		case INS_TYPE_J:
			decoded_instruction->rd = DECODE_GET_RD(encoded_instruction);
			decoded_instruction->imm = DECODE_GET_J_IMM(encoded_instruction);
			break;
		default:
			return false;
	}

	return true;
}

bool cpu_decode_and_cache(emulator_t* emu, guest_paddr instruction_addr, ins_t** decoded_instruction) {
	assert(!emu->cpu.dynarec_enabled);
	assert((instruction_addr & 3) == 0);

	size_t cache_index = (instruction_addr >> 2) & emu->cpu.instruction_cache_mask;
	cached_ins_t* cached_instruction = &emu->cpu.instruction_cache.as_cached_ins[cache_index];
	*decoded_instruction = &cached_instruction->decoded_instruction;

	if (cached_instruction->decoded_instruction.type != INS_TYPE_INVALID &&
	    cached_instruction->tag == instruction_addr) {
		return true;
	}

	uint8_t exception_code;
	guest_reg exception_tval;
	uint32_t encoded_instruction = emu_r32_ins(emu, instruction_addr, &exception_code, &exception_tval);
	if (exception_code != (uint8_t)-1) {
		cpu_throw_exception(emu, exception_code, exception_tval);
		return false;
	}
	if (!cpu_decode(encoded_instruction, &cached_instruction->decoded_instruction)) {
		cached_instruction->decoded_instruction.type = INS_TYPE_INVALID;
		return false;
	}
	cached_instruction->tag = instruction_addr;
	return true;
}

bool cpu_invalidate_instruction_cache(emulator_t* emu, guest_paddr addr) {
#ifdef RISCV_EMULATOR_DYNAREC_X86_64_SUPPORT
	if (emu->cpu.dynarec_enabled) {
		size_t cache_index = (addr >> 2) & emu->cpu.instruction_cache_mask;
		dr_ins_t* cached_instruction = &emu->cpu.instruction_cache.as_dr_ins[cache_index];

		if (cached_instruction->native_code != NULL &&
		    cached_instruction->tag == (addr & ~3)) {
			uintptr_t page_base = (uintptr_t)cached_instruction->native_code & DYNAREC_PAGE_MASK;
			munmap((void*)page_base, DYNAREC_PAGE_SIZE);

			guest_reg block_entry = cached_instruction->block_entry;
			for (guest_reg pc = block_entry;; pc += 4) {
				size_t index = (pc >> 2) & emu->cpu.instruction_cache_mask;
				dr_ins_t* entry = &emu->cpu.instruction_cache.as_dr_ins[index];
				if (entry->block_entry == block_entry &&
				    ((uintptr_t)entry->native_code & DYNAREC_PAGE_MASK) == page_base) {
					entry->native_code = NULL;
					entry->tag = entry->block_entry = 0;
				} else {
					break;
				}
			}
			return true;
		} else {
			return false;
		}
	}
#else
	assert(!emu->cpu.dynarec_enabled);
#endif

	size_t cache_index = (addr >> 2) & emu->cpu.instruction_cache_mask;
	cached_ins_t* cached_instruction = &emu->cpu.instruction_cache.as_cached_ins[cache_index];
	if (cached_instruction->decoded_instruction.type != INS_TYPE_INVALID &&
	    cached_instruction->tag == (addr & ~3)) {
		cached_instruction->decoded_instruction.type = INS_TYPE_INVALID;
		return true;
	} else {
		return false;
	}
}

void cpu_flush_instruction_cache(emulator_t* emu) {
	size_t instruction_cache_size = emu->cpu.instruction_cache_mask + 1;
	emu->cpu.tlb_or_cache_flush_pending = true;

#ifdef RISCV_EMULATOR_DYNAREC_X86_64_SUPPORT
	if (emu->cpu.dynarec_enabled) {
		for (size_t i = 0; i < instruction_cache_size; i++) {
			dr_ins_t* cached_instruction = &emu->cpu.instruction_cache.as_dr_ins[i];
			if (cached_instruction->native_code != NULL) {
				cpu_invalidate_instruction_cache(emu, cached_instruction->tag);
			}
		}
		return;
	}
#else
	assert(!emu->cpu.dynarec_enabled);
#endif

	for (size_t i = 0; i < instruction_cache_size; i++) {
		cached_ins_t* cached_instruction = &emu->cpu.instruction_cache.as_cached_ins[i];
		if (cached_instruction->decoded_instruction.type != INS_TYPE_INVALID) {
			cpu_invalidate_instruction_cache(emu, cached_instruction->tag);
		}
	}
}

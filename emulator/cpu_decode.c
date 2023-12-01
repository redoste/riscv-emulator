#include <assert.h>
#include <stddef.h>
#include <stdint.h>

#include "cpu.h"
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
	assert((instruction_addr & 3) == 0);

	size_t cache_index = (instruction_addr >> 2) & emu->cpu.instruction_cache_mask;
	cached_ins_t* cached_instruction = &emu->cpu.instruction_cache[cache_index];
	*decoded_instruction = &cached_instruction->decoded_instruction;

	if (cached_instruction->decoded_instruction.type != INS_TYPE_INVALID &&
	    cached_instruction->tag == instruction_addr) {
		return true;
	}

	uint32_t encoded_instruction = emu_r32(emu, instruction_addr);
	if (!cpu_decode(encoded_instruction, &cached_instruction->decoded_instruction)) {
		cached_instruction->decoded_instruction.type = INS_TYPE_INVALID;
		return false;
	}
	cached_instruction->tag = instruction_addr;
	return true;
}

void cpu_invalidate_instruction_cache(emulator_t* emu, guest_paddr addr) {
	size_t cache_index = (addr >> 2) & emu->cpu.instruction_cache_mask;
	cached_ins_t* cached_instruction = &emu->cpu.instruction_cache[cache_index];
	if (cached_instruction->decoded_instruction.type != INS_TYPE_INVALID &&
	    cached_instruction->tag == (addr & ~3)) {
		cached_instruction->decoded_instruction.type = INS_TYPE_INVALID;
	}
}

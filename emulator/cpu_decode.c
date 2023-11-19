#include <stdint.h>

#include "cpu.h"
#include "isa.h"

bool cpu_decode(uint32_t encoded_instruction, ins_t* decoded_instruction) {
	decoded_instruction->opcode = DECODE_GET_OPCODE(encoded_instruction);
	// RISC-V I opcodes always have their lower two bits set
	if ((decoded_instruction->opcode & 3) != 3) {
		return false;
	}
	decoded_instruction->type = INS_TYPES[decoded_instruction->opcode >> 2];

	switch (decoded_instruction->type) {
		case INS_TYPE_R:
			decoded_instruction->rd = DECODE_GET_RD(encoded_instruction);
			decoded_instruction->funct3 = DECODE_GET_F3(encoded_instruction);
			decoded_instruction->rs1 = DECODE_GET_RS1(encoded_instruction);
			decoded_instruction->rs2 = DECODE_GET_RS2(encoded_instruction);
			decoded_instruction->funct7 = DECODE_GET_F7(encoded_instruction);
			break;
		case INS_TYPE_I:
			decoded_instruction->rd = DECODE_GET_RD(encoded_instruction);
			decoded_instruction->funct3 = DECODE_GET_F3(encoded_instruction);
			decoded_instruction->rs1 = DECODE_GET_RS1(encoded_instruction);
			decoded_instruction->imm = DECODE_GET_I_IMM(encoded_instruction);
			break;
		case INS_TYPE_S:
			decoded_instruction->funct3 = DECODE_GET_F3(encoded_instruction);
			decoded_instruction->rs1 = DECODE_GET_RS1(encoded_instruction);
			decoded_instruction->rs2 = DECODE_GET_RS2(encoded_instruction);
			decoded_instruction->imm = DECODE_GET_S_IMM(encoded_instruction);
			break;
		case INS_TYPE_B:
			decoded_instruction->funct3 = DECODE_GET_F3(encoded_instruction);
			decoded_instruction->rs1 = DECODE_GET_RS1(encoded_instruction);
			decoded_instruction->rs2 = DECODE_GET_RS2(encoded_instruction);
			decoded_instruction->imm = DECODE_GET_B_IMM(encoded_instruction);
			break;
		case INS_TYPE_U:
			decoded_instruction->rd = DECODE_GET_RD(encoded_instruction);
			decoded_instruction->imm = DECODE_GET_U_IMM(encoded_instruction);
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

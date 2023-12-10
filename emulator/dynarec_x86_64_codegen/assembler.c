#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"

static size_t asm_x86_emit_ins(uint8_t* out, size_t out_size, x86_reloc_type_t reloc_type, size_t* reloc_pos, const x86_ins_t* ins) {
	/* x86-64 Instruction format :
	 *
	 * [Legacy prefixes]
	 * [REX prefix]
	 * [Opcode]
	 * [ModR/M]
	 * [SIB]
	 * [Disp]
	 * [Imm]
	 */

	size_t pos = 0;
#define EMIT_BYTE(byte)                \
	do {                           \
		if (pos >= out_size) { \
			return 0;      \
		}                      \
		out[pos++] = byte;     \
	} while (0)

	/* We don't support legacy prefixes (e.g. used for segment override, operand
	 * or address size override)
	 */

	/* REX Prefix :
	 * 0b0100WRXB
	 * W : 64-bit override
	 * R : ModR/M reg extension
	 * X : SIB index extension
	 * B : ModR/M r/m extension
	 */
	if (ins->rex_w || ins->reg >= 8 || ins->rm >= 8) {
		uint8_t rex = 0x40 |
			      ((ins->rex_w & 1) << 3) |
			      (((ins->reg >> 3) & 1) << 2) |
			      (((ins->rm >> 3) & 1) << 0);
		EMIT_BYTE(rex);
	}

	// Opcode
	uint64_t opcode = ins->opcode;
	if (ins->addr_mode == X86_AM_REG) {
		opcode |= ins->rm & 7;
	}
	for (size_t i = 0; i < ins->opcode_size; i++) {
		EMIT_BYTE(opcode & 0xff);
		opcode >>= 8;
	}

	if (ins->addr_mode != X86_AM_REG) {
		/* ModR/M :
		 * 0bMMRRRXXX
		 * M : addressing mode
		 * R : reg
		 * X : r/m
		 */
		uint8_t rm = ((ins->reg & 7) << 3) |
			     (ins->rm & 7);
		size_t disp_size = 0;
		switch (ins->addr_mode) {
			case X86_AM_RM:
				rm |= 0xc0;
				break;
			case X86_AM_DEREF_RM:
				rm |= 0x00;
				break;
			case X86_AM_DEREF_RM_DISP_8:
				rm |= 0x40;
				disp_size = 1;
				break;
			case X86_AM_DEREF_RM_DISP_32:
				rm |= 0x80;
				disp_size = 4;
				break;
			default:
				fprintf(stderr, "Unexpected x86-64 addressing mode\n");
				abort();
		}
		EMIT_BYTE(rm);

		// We don't support SIB for now (e.g. [rdi+rcx*8])
		if (ins->addr_mode != X86_AM_RM && (ins->rm == RSP || ins->rm == R12)) {
			// RSP and R12 are used to announce SIB
			fprintf(stderr,
				"Using RSP or R12 in R/M conflics with Scale/Index/Base deref\n"
				"SIB isn't supported for now, if possible try to use other registers\n");
			abort();
		}

		// Disp
		if (reloc_type == X86_RELOC_DISP) {
			assert(reloc_pos != NULL);
			*reloc_pos = pos;
		}
		int64_t disp = ins->disp;
		for (size_t i = 0; i < disp_size; i++) {
			EMIT_BYTE(disp & 0xff);
			disp >>= 8;
		}
	}

	// Immediate
	if (reloc_type == X86_RELOC_IMM) {
		assert(reloc_pos != NULL);
		*reloc_pos = pos;
	}
	int64_t imm = ins->imm;
	for (size_t i = 0; i < ins->imm_size; i++) {
		EMIT_BYTE(imm & 0xff);
		imm >>= 8;
	}

#undef EMIT_BYTE
	return pos;
}

static bool asm_x86_is_operand_encoding_compatible(x86_operand_t op_type, x86_ins_encoding_operand_type_t encoding_type, int64_t imm) {
	switch (encoding_type) {
		case X86_OPERAND_ENCODING_NONE:
			return op_type == X86_OPERAND_NONE;
		case X86_OPERAND_ENCODING_RM64:
			return op_type == X86_OPERAND_REG ||
			       op_type == X86_OPERAND_DEREF ||
			       op_type == X86_OPERAND_DISP ||
			       op_type == X86_OPERAND_RELOC_DISP8;
		case X86_OPERAND_ENCODING_R64:
			return op_type == X86_OPERAND_REG;
		case X86_OPERAND_ENCODING_IMM8:
			return op_type == X86_OPERAND_IMM && imm <= INT8_MAX && imm >= INT8_MIN;
		case X86_OPERAND_ENCODING_IMM32:
			return (op_type == X86_OPERAND_IMM && imm <= INT32_MAX && imm >= INT32_MIN) ||
			       op_type == X86_OPERAND_RELOC_IMM32;
		case X86_OPERAND_ENCODING_IMM64:
			return (op_type == X86_OPERAND_IMM && imm <= INT64_MAX && imm >= INT64_MIN) ||
			       op_type == X86_OPERAND_RELOC_IMM32;
		default:
			return false;
	}
}

// TODO : support having 2 relocations per instruction
size_t asm_x86_ins(uint8_t* out, size_t out_size, size_t* reloc_pos, x86_mnemonic_t mnemonic, x86_operand_t dst, x86_operand_t src) {
	x86_ins_t ins = {0};
	x86_reloc_type_t reloc_type = X86_RELOC_NONE;

	x86_operand_type_t dst_op_type = OP_GET_TYPE(dst);
	int64_t dst_imm = OP_GET_IMM(dst);
	x86_reg_t dst_reg = OP_GET_REG(dst);
	int32_t dst_disp = OP_GET_DISP(dst);

	x86_operand_type_t src_op_type = OP_GET_TYPE(src);
	int64_t src_imm = OP_GET_IMM(src);
	x86_reg_t src_reg = OP_GET_REG(src);
	int32_t src_disp = OP_GET_DISP(src);

	const x86_ins_encoding_t* encoding = X86_INS_ENCODINGS[mnemonic];
	if (encoding == NULL) {
		return 0;
	}

	for (; encoding->opcode_size > 0; encoding++) {
		if (!asm_x86_is_operand_encoding_compatible(dst_op_type, encoding->dst, dst_imm) ||
		    !asm_x86_is_operand_encoding_compatible(src_op_type, encoding->src, src_imm)) {
			continue;
		}

		// Opcode
		ins.opcode = encoding->opcode;
		ins.opcode_size = encoding->opcode_size;
		ins.rex_w = encoding->rex_w;

		// Addressing mode
		if (dst_op_type == X86_OPERAND_DEREF || src_op_type == X86_OPERAND_DEREF) {
			ins.addr_mode = X86_AM_DEREF_RM;
		} else if ((dst_op_type == X86_OPERAND_DISP && dst_disp <= INT8_MAX && dst_disp >= INT8_MIN) ||
			   (src_op_type == X86_OPERAND_DISP && src_disp <= INT8_MAX && src_disp >= INT8_MIN) ||
			   src_op_type == X86_OPERAND_RELOC_DISP8 ||
			   dst_op_type == X86_OPERAND_RELOC_DISP8) {
			ins.addr_mode = X86_AM_DEREF_RM_DISP_8;
		} else if ((dst_op_type == X86_OPERAND_DISP && dst_disp <= INT32_MAX && dst_disp >= INT32_MIN) ||
			   (src_op_type == X86_OPERAND_DISP && src_disp <= INT32_MAX && src_disp >= INT32_MIN)) {
			ins.addr_mode = X86_AM_DEREF_RM_DISP_32;
		} else if (encoding->dst == X86_OPERAND_ENCODING_RM64 || encoding->src == X86_OPERAND_ENCODING_RM64) {
			ins.addr_mode = X86_AM_RM;
		} else {
			ins.addr_mode = X86_AM_REG;
		}

		// Registers
		if (ins.addr_mode == X86_AM_REG) {
			if (src_op_type == X86_OPERAND_REG) {
				ins.rm = src_reg;
			} else if (dst_op_type == X86_OPERAND_REG) {
				ins.rm = dst_reg;
			}
		} else {
			if (encoding->dst == X86_OPERAND_ENCODING_RM64) {
				ins.rm = dst_reg;
				ins.reg = encoding->src == X86_OPERAND_ENCODING_R64 ? src_reg : encoding->rm_filler;
			} else if (encoding->src == X86_OPERAND_ENCODING_RM64) {
				ins.rm = src_reg;
				ins.reg = encoding->dst == X86_OPERAND_ENCODING_R64 ? dst_reg : encoding->rm_filler;
			}
		}

		// Disp
		if (dst_op_type == X86_OPERAND_DISP) {
			ins.disp = dst_disp;
		} else if (src_op_type == X86_OPERAND_DISP) {
			ins.disp = src_disp;
		}

		// Immediate
		if (encoding->src == X86_OPERAND_ENCODING_IMM8) {
			ins.imm_size = 1;
			ins.imm = src_op_type == X86_OPERAND_IMM ? src_imm : 0;
		} else if (encoding->src == X86_OPERAND_ENCODING_IMM32) {
			ins.imm_size = 4;
			ins.imm = src_op_type == X86_OPERAND_IMM ? src_imm : 0;
		} else if (encoding->src == X86_OPERAND_ENCODING_IMM64) {
			ins.imm_size = 8;
			ins.imm = src_op_type == X86_OPERAND_IMM ? src_imm : 0;
		} else if (encoding->dst == X86_OPERAND_ENCODING_IMM8) {
			ins.imm_size = 1;
			ins.imm = dst_op_type == X86_OPERAND_IMM ? dst_imm : 0;
		} else if (encoding->dst == X86_OPERAND_ENCODING_IMM32) {
			ins.imm_size = 4;
			ins.imm = dst_op_type == X86_OPERAND_IMM ? dst_imm : 0;
		} else if (encoding->dst == X86_OPERAND_ENCODING_IMM64) {
			ins.imm_size = 8;
			ins.imm = dst_op_type == X86_OPERAND_IMM ? dst_imm : 0;
		}

		// Relocations
		if (src_op_type == X86_OPERAND_RELOC_DISP8 || dst_op_type == X86_OPERAND_RELOC_DISP8) {
			reloc_type = X86_RELOC_DISP;
		} else if (src_op_type == X86_OPERAND_RELOC_IMM32 || dst_op_type == X86_OPERAND_RELOC_IMM32) {
			reloc_type = X86_RELOC_IMM;
		}

		return asm_x86_emit_ins(out, out_size, reloc_type, reloc_pos, &ins);
	}
	return 0;
}

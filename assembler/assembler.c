#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"
#include "diag.h"
#include "isa.h"
#include "lexer.h"

/* NOTE : it doesn't look nice to have bound checks in the "parse" functions
 *        but this way we can point to the corrent token in the diagnostic
 *        in case of error
 */

static bool assembler_bound_check_imm(const token_t* token, int64_t imm, size_t bits, bool even) {
	assert(bits < 64);

	int64_t lower_bound = -(1 << (bits - 1));
	int64_t upper_bound = (1 << (bits - 1)) - (even ? 2 : 1);
	if (imm < lower_bound || imm > upper_bound) {
		diag_error(token->pos, "immediate out of range [%ld:%ld]\n", lower_bound, upper_bound);
		return false;
	} else if (even && (imm & 1) != 0) {
		diag_error(token->pos, "immediate should be even\n");
		return false;
	} else {
		return true;
	}
}

#define RETURN_IF_LEXER_UNEXPECTED(lexer, token, type)                       \
	do {                                                                 \
		if (!lexer_next_expected((lexer), (token), (type), false)) { \
			return false;                                        \
		}                                                            \
	} while (0)

static bool assembler_parse_r_ins(lexer_t* lexer, reg_t* rd, reg_t* rs1, reg_t* rs2) {
	token_t token;

	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
	*rd = token.as_reg_operand;
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_COMMA);
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
	*rs1 = token.as_reg_operand;
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_COMMA);
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
	*rs2 = token.as_reg_operand;
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_EOI);

	return true;
}

static bool assembler_parse_i_ins(lexer_t* lexer, reg_t* rd, reg_t* rs1, int64_t* imm) {
	token_t token;

	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
	*rd = token.as_reg_operand;
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_COMMA);
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
	*rs1 = token.as_reg_operand;
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_COMMA);
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_INT_LITERAL);
	*imm = token.as_int_literal;
	if (!assembler_bound_check_imm(&token, *imm, 12, false)) {
		return false;
	}
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_EOI);

	return true;
}

static bool assembler_parse_s_ins(lexer_t* lexer, reg_t* rs1, reg_t* rs2, int64_t* imm) {
	token_t token;

	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
	*rs2 = token.as_reg_operand;
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_COMMA);
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_INT_LITERAL);
	*imm = token.as_int_literal;
	if (!assembler_bound_check_imm(&token, *imm, 12, false)) {
		return false;
	}
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_DEREF_OPERAND);
	*rs1 = token.as_reg_deref_operand;
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_EOI);

	return true;
}

static bool assembler_parse_b_ins(lexer_t* lexer, reg_t* rs1, reg_t* rs2, int64_t* imm) {
	token_t token;

	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
	*rs1 = token.as_reg_operand;
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_COMMA);
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
	*rs2 = token.as_reg_operand;
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_COMMA);
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_INT_LITERAL);
	*imm = token.as_int_literal;
	if (!assembler_bound_check_imm(&token, *imm, 13, true)) {
		return false;
	}
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_EOI);

	return true;
}

static bool assembler_parse_j_ins(lexer_t* lexer, reg_t* rd, int64_t* imm) {
	token_t token;

	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
	*rd = token.as_reg_operand;
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_COMMA);
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_INT_LITERAL);
	*imm = token.as_int_literal;
	if (!assembler_bound_check_imm(&token, *imm, 21, true)) {
		return false;
	}
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_EOI);

	return true;
}

static bool assembler_parse_u_ins(lexer_t* lexer, reg_t* rd, int64_t* imm) {
	token_t token;

	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
	*rd = token.as_reg_operand;
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_COMMA);
	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_INT_LITERAL);
	int64_t imm_high_bits = token.as_int_literal;

	const int64_t upper_bound = (1 << 20) - 1;
	if (imm_high_bits < 0 || imm_high_bits > upper_bound) {
		diag_error(token.pos, "immediate out of range [0:%ld]\n", upper_bound);
		return false;
	}
	*imm = imm_high_bits << 12;

	RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_EOI);

	return true;
}

static bool assembler_assemble_pseudo_instruction(ins_mnemonic_t mnemonic, lexer_t* lexer, uint32_t* instruction) {
	switch (mnemonic) {
		case INS_J: {
			token_t token;
			RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_INT_LITERAL);
			int64_t imm = token.as_int_literal;
			if (!assembler_bound_check_imm(&token, imm, 21, true)) {
				return false;
			}
			RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_EOI);
			// emit `jal x0, imm`
			*instruction = ENCODE_J_INSTRUCTION(OPCODE_JAL, 0, imm);
			return true;
		};
		case INS_LI: {
			token_t token;
			RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
			reg_t rd = token.as_reg_operand;
			RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_COMMA);
			RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_INT_LITERAL);
			int64_t imm = token.as_int_literal;
			if (!assembler_bound_check_imm(&token, imm, 12, false)) {
				return false;
			}
			RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_EOI);
			// emit `addi rd, x0, imm`
			*instruction = ENCODE_I_INSTRUCTION(OPCODE_OP_IMM, F3_ADD, rd, 0, imm);
			return true;
		};
		case INS_MV: {
			token_t token;
			RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
			reg_t rd = token.as_reg_operand;
			RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_COMMA);
			RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_REG_OPERAND);
			reg_t rs = token.as_reg_operand;
			RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_EOI);
			// emit `addi rd, rs, 0`
			*instruction = ENCODE_I_INSTRUCTION(OPCODE_OP_IMM, F3_ADD, rd, rs, 0);
			return true;
		};
		case INS_ECALL: {
			token_t token;
			RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_EOI);
			// emit `ecall x0, x0, 0`
			*instruction = ENCODE_I_INSTRUCTION(OPCODE_SYSTEM, F3_ECALL, 0, 0, F12_ECALL);
			return true;
		};
		case INS_EBREAK: {
			token_t token;
			RETURN_IF_LEXER_UNEXPECTED(lexer, &token, TT_EOI);
			// emit `ebreak x0, x0, 1`
			*instruction = ENCODE_I_INSTRUCTION(OPCODE_SYSTEM, F3_EBREAK, 0, 0, F12_EBREAK);
			return true;
		};

		default:
			fprintf(stderr, "ICE : Invalid pseudo-instruction mnemonic\n");
			abort();
			break;
	}
}

bool assembler_assemble_instruction(ins_mnemonic_t mnemonic, lexer_t* lexer, uint32_t* instruction) {
	reg_t rs1, rs2, rd;
	int64_t imm;

	switch (mnemonic) {
#define X_R(MNEMONIC, O, F3, F7)                                                    \
	case INS_##MNEMONIC:                                                        \
		if (!assembler_parse_r_ins(lexer, &rd, &rs1, &rs2)) {               \
			return false;                                               \
		}                                                                   \
		*instruction = ENCODE_R_INSTRUCTION((O), (F3), (F7), rd, rs1, rs2); \
		return true;
#define X_I(MNEMONIC, O, F3)                                                  \
	case INS_##MNEMONIC:                                                  \
		if (!assembler_parse_i_ins(lexer, &rd, &rs1, &imm)) {         \
			return false;                                         \
		}                                                             \
		*instruction = ENCODE_I_INSTRUCTION((O), (F3), rd, rs1, imm); \
		return true;
#define X_I_S(MNEMONIC, O, F3)                                                      \
	case INS_##MNEMONIC:                                                        \
		/* NOTE : It's safe to use assembler_parse_s_ins for I-type */      \
		/*        instructions that parses like S-type ones (e.g. loads) */ \
		/*        as both I-type and S-type instructions have 12 bits */    \
		/*        immediates. */                                            \
		if (!assembler_parse_s_ins(lexer, &rs1, &rd, &imm)) {               \
			return false;                                               \
		}                                                                   \
		*instruction = ENCODE_I_INSTRUCTION((O), (F3), rd, rs1, imm);       \
		return true;
#define X_S(MNEMONIC, O, F3)                                                   \
	case INS_##MNEMONIC:                                                   \
		if (!assembler_parse_s_ins(lexer, &rs1, &rs2, &imm)) {         \
			return false;                                          \
		}                                                              \
		*instruction = ENCODE_S_INSTRUCTION((O), (F3), rs1, rs2, imm); \
		return true;
#define X_B(MNEMONIC, O, F3)                                                   \
	case INS_##MNEMONIC:                                                   \
		if (!assembler_parse_b_ins(lexer, &rs1, &rs2, &imm)) {         \
			return false;                                          \
		}                                                              \
		*instruction = ENCODE_B_INSTRUCTION((O), (F3), rs1, rs2, imm); \
		return true;
#define X_U(MNEMONIC, O)                                           \
	case INS_##MNEMONIC:                                       \
		if (!assembler_parse_u_ins(lexer, &rd, &imm)) {    \
			return false;                              \
		}                                                  \
		*instruction = ENCODE_U_INSTRUCTION((O), rd, imm); \
		return true;
#define X_J(MNEMONIC, O)                                           \
	case INS_##MNEMONIC:                                       \
		if (!assembler_parse_j_ins(lexer, &rd, &imm)) {    \
			return false;                              \
		}                                                  \
		*instruction = ENCODE_J_INSTRUCTION((O), rd, imm); \
		return true;
#define X_P(MNEMONIC)        \
	case INS_##MNEMONIC: \
		return assembler_assemble_pseudo_instruction(mnemonic, lexer, instruction);

		X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_I_S
#undef X_S
#undef X_B
#undef X_U
#undef X_J
#undef X_P

		case INS_COUNT:
		default:
			fprintf(stderr, "ICE : Invalid mnemonic\n");
			abort();
			break;
	}
}

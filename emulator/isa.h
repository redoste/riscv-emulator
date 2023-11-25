#ifndef ISA_H
#define ISA_H

#include <stdint.h>

#include <rv64_isa.h>

/* NOTE : we pack the enumeration to make sure ins_type_t values are encoded on one byte and
 *        prevent the INS_TYPES array to have 3/4 of its space wasted
 */
typedef enum __attribute__((packed)) ins_type_t {
	INS_TYPE_INVALID = 0,

	INS_TYPE_R,
	INS_TYPE_I,
	INS_TYPE_S,
	INS_TYPE_B,
	INS_TYPE_U,
	INS_TYPE_J,
} ins_type_t;

typedef struct ins_t {
	ins_type_t type;

	uint8_t opcode;
	uint8_t funct3;
	uint8_t funct7;

	reg_t rd;
	reg_t rs1;
	reg_t rs2;

	int64_t imm;
} ins_t;

/* X_INSTRUCTIONS : X-macro storing informations about all the instructions
 *                  the emulator can emulate
 *     X_R(MNEMONIC, OPCODE, FUNCT3, FUNCT7, EXPR) : R-type instruction
 *     X_I(MNEMONIC, OPCODE, FUNCT3, EXPR)         : I-type instruction
 *     X_S(MNEMONIC, OPCODE, FUNCT3, EXPR)         : S-type instruction
 *     X_B(MNEMONIC, OPCODE, FUNCT3, EXPR)         : B-type instruction
 *     X_U(MNEMONIC, OPCODE, EXPR)                 : U-type instruction
 *     X_J(MNEMONIC, OPCODE, EXPR)                 : J-type instruction
 */
#define X_INSTRUCTIONS                                                                                                                                     \
	X_R(ADD, OPCODE_OP, F3_ADD, F7_ADD, *rd = *rs1 + *rs2)                                                                                             \
	X_R(SUB, OPCODE_OP, F3_SUB, F7_SUB, *rd = *rs1 - *rs2)                                                                                             \
	X_R(SLL, OPCODE_OP, F3_SLL, F7_SLL, *rd = *rs1 << (*rs2 & 0x3f))                                                                                   \
	X_R(SLT, OPCODE_OP, F3_SLT, F7_SLT, *rd = (*rs1s < *rs2s) ? 1 : 0)                                                                                 \
	X_R(SLTU, OPCODE_OP, F3_SLTU, F7_SLTU, *rd = (*rs1 < *rs2) ? 1 : 0)                                                                                \
	X_R(XOR, OPCODE_OP, F3_XOR, F7_XOR, *rd = *rs1 ^ *rs2)                                                                                             \
	X_R(SRL, OPCODE_OP, F3_SRL, F7_SRL, *rd = *rs1 >> (*rs2 & 0x3f))                                                                                   \
	X_R(SRA, OPCODE_OP, F3_SRA, F7_SRA, *rd = *rs1s >> (*rs2 & 0x3f))                                                                                  \
	X_R(OR, OPCODE_OP, F3_OR, F7_OR, *rd = *rs1 | *rs2)                                                                                                \
	X_R(AND, OPCODE_OP, F3_AND, F7_AND, *rd = *rs1 & *rs2)                                                                                             \
                                                                                                                                                           \
	X_R(ADDW, OPCODE_OP_32, F3_ADD, F7_ADD, *rds = (guest_word_signed)(*rs1w + *rs2w))                                                                 \
	X_R(SUBW, OPCODE_OP_32, F3_SUB, F7_SUB, *rds = (guest_word_signed)(*rs1w - *rs2w))                                                                 \
	X_R(SLLW, OPCODE_OP_32, F3_SLL, F7_SLL, *rds = (guest_word_signed)(*rs1w << (*rs2 & 0x1f)))                                                        \
	X_R(SRLW, OPCODE_OP_32, F3_SRL, F7_SRL, *rds = (guest_word_signed)(*rs1w >> (*rs2 & 0x1f)))                                                        \
	X_R(SRAW, OPCODE_OP_32, F3_SRA, F7_SRA, *rds = (guest_word_signed)(*rs1ws >> (*rs2 & 0x1f)))                                                       \
                                                                                                                                                           \
	X_I(LB, OPCODE_LOAD, F3_LB, *rds = (int8_t)emu_r8(emu, *rs1 + imm))                                                                                \
	X_I(LH, OPCODE_LOAD, F3_LH, *rds = (int16_t)emu_r16(emu, *rs1 + imm))                                                                              \
	X_I(LW, OPCODE_LOAD, F3_LW, *rds = (int32_t)emu_r32(emu, *rs1 + imm))                                                                              \
	X_I(LD, OPCODE_LOAD, F3_LD, *rd = emu_r64(emu, *rs1 + imm))                                                                                        \
	X_I(LBU, OPCODE_LOAD, F3_LBU, *rd = (uint8_t)emu_r8(emu, *rs1 + imm))                                                                              \
	X_I(LHU, OPCODE_LOAD, F3_LHU, *rd = (uint16_t)emu_r16(emu, *rs1 + imm))                                                                            \
	X_I(LWU, OPCODE_LOAD, F3_LWU, *rd = (uint32_t)emu_r32(emu, *rs1 + imm))                                                                            \
                                                                                                                                                           \
	X_I(FENCE, OPCODE_MISC_MEM, F3_FENCE, /* nop */)                                                                                                   \
                                                                                                                                                           \
	X_I(ADDI, OPCODE_OP_IMM, F3_ADD, *rd = *rs1 + imm)                                                                                                 \
	X_I(SLLI, OPCODE_OP_IMM, F3_SLL, *rd = *rs1 << (imm & 0x3f))                                                                                       \
	X_I(SLTI, OPCODE_OP_IMM, F3_SLT, *rd = (*rs1s < imm) ? 1 : 0)                                                                                      \
	X_I(SLTIU, OPCODE_OP_IMM, F3_SLTU, *rd = (*rs1 < (uint64_t)imm) ? 1 : 0)                                                                           \
	X_I(XORI, OPCODE_OP_IMM, F3_XOR, *rd = *rs1 ^ imm)                                                                                                 \
	X_I(SRLI, OPCODE_OP_IMM, F3_SRL, *rd = (imm & F12_SRA) ? (guest_reg)(*rs1s >> (imm & 0x3f)) : (*rs1 >> (imm & 0x3f)))                              \
	X_I(ORI, OPCODE_OP_IMM, F3_OR, *rd = *rs1 | imm)                                                                                                   \
	X_I(ANDI, OPCODE_OP_IMM, F3_AND, *rd = *rs1 & imm)                                                                                                 \
                                                                                                                                                           \
	X_I(ADDIW, OPCODE_OP_IMM_32, F3_ADD, *rds = (guest_word_signed)(*rs1w + imm))                                                                      \
	X_I(SLLIW, OPCODE_OP_IMM_32, F3_SLL, *rds = (guest_word_signed)(*rs1w << (imm & 0x1f)))                                                            \
	X_I(SRLIW, OPCODE_OP_IMM_32, F3_SRL, *rds = (guest_word_signed)((imm & F12_SRA) ? (guest_word)(*rs1ws >> (imm & 0x1f)) : (*rs1w >> (imm & 0x1f)))) \
                                                                                                                                                           \
	X_I(                                                                                                                                               \
		JALR, OPCODE_JALR, F3_JALR, do {                                                                                                           \
			guest_reg old_pc = cpu->pc + 4;                                                                                                    \
			cpu->pc = (*rs1 + imm) & 0xfffffffffffffffe;                                                                                       \
			*rd = old_pc;                                                                                                                      \
			cpu->jump_pending = true;                                                                                                          \
		} while (0))                                                                                                                               \
                                                                                                                                                           \
	X_I(ECALL, OPCODE_SYSTEM, F3_ECALL, (imm == F12_EBREAK) ? emu_ebreak(emu) : emu_ecall(emu))                                                        \
                                                                                                                                                           \
	X_S(SB, OPCODE_STORE, F3_SB, emu_w8(emu, *rs1 + imm, *rs2))                                                                                        \
	X_S(SH, OPCODE_STORE, F3_SH, emu_w16(emu, *rs1 + imm, *rs2))                                                                                       \
	X_S(SW, OPCODE_STORE, F3_SW, emu_w32(emu, *rs1 + imm, *rs2))                                                                                       \
	X_S(SD, OPCODE_STORE, F3_SD, emu_w64(emu, *rs1 + imm, *rs2))                                                                                       \
                                                                                                                                                           \
	X_B(                                                                                                                                               \
		BEQ, OPCODE_BRANCH, F3_BEQ, do {                                                                                                           \
			if (*rs1 == *rs2) {                                                                                                                \
				cpu->pc += imm;                                                                                                            \
				cpu->jump_pending = true;                                                                                                  \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_B(                                                                                                                                               \
		BNE, OPCODE_BRANCH, F3_BNE, do {                                                                                                           \
			if (*rs1 != *rs2) {                                                                                                                \
				cpu->pc += imm;                                                                                                            \
				cpu->jump_pending = true;                                                                                                  \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_B(                                                                                                                                               \
		BLT, OPCODE_BRANCH, F3_BLT, do {                                                                                                           \
			if (*rs1s < *rs2s) {                                                                                                               \
				cpu->pc += imm;                                                                                                            \
				cpu->jump_pending = true;                                                                                                  \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_B(                                                                                                                                               \
		BGE, OPCODE_BRANCH, F3_BGE, do {                                                                                                           \
			if (*rs1s >= *rs2s) {                                                                                                              \
				cpu->pc += imm;                                                                                                            \
				cpu->jump_pending = true;                                                                                                  \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_B(                                                                                                                                               \
		BLTU, OPCODE_BRANCH, F3_BLTU, do {                                                                                                         \
			if (*rs1 < *rs2) {                                                                                                                 \
				cpu->pc += imm;                                                                                                            \
				cpu->jump_pending = true;                                                                                                  \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_B(                                                                                                                                               \
		BGEU, OPCODE_BRANCH, F3_BGEU, do {                                                                                                         \
			if (*rs1 >= *rs2) {                                                                                                                \
				cpu->pc += imm;                                                                                                            \
				cpu->jump_pending = true;                                                                                                  \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
                                                                                                                                                           \
	X_U(AUIPC, OPCODE_AUIPC, *rd = cpu->pc + imm)                                                                                                      \
	X_U(LUI, OPCODE_LUI, *rd = imm)                                                                                                                    \
                                                                                                                                                           \
	X_J(                                                                                                                                               \
		JAL, OPCODE_JAL, do {                                                                                                                      \
			*rd = cpu->pc + 4;                                                                                                                 \
			cpu->pc += imm;                                                                                                                    \
			cpu->jump_pending = true;                                                                                                          \
		} while (0))

extern const ins_type_t INS_TYPES[0x20];

#endif

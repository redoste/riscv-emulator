#ifndef ISA_H
#define ISA_H

#include <stdbool.h>
#include <stdint.h>

#include <rv64_isa.h>

/* ins_type_t : enumeration of the types of RISC-V instructions
 * NOTE : we pack the enumeration to make sure ins_type_t values are encoded on one byte and
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

/* ins_t : structure representing a decoded RISC-V instruction
 */
typedef struct ins_t {
	ins_type_t type;

	uint16_t opcode_switch;

	reg_t rd;
	reg_t rs1;
	reg_t rs2;

	int64_t imm;
} ins_t;

/* cached_ins_t : structure representing a decoded RISC-V instruction cached in
 *                the CPU instruction cache
 */
typedef struct cached_ins_t {
	guest_paddr tag;
	ins_t decoded_instruction;
} cached_ins_t;

/* X_INSTRUCTIONS : X-macro storing informations about all the instructions
 *                  the emulator can emulate
 *     X_R(MNEMONIC, OPCODE, FUNCT3, FUNCT7, EXPR) : R-type instruction
 *     X_I(MNEMONIC, OPCODE, FUNCT3, EXPR)         : I-type instruction
 *     X_S(MNEMONIC, OPCODE, FUNCT3, EXPR)         : S-type instruction
 *     X_B(MNEMONIC, OPCODE, FUNCT3, EXPR)         : B-type instruction
 *     X_U(MNEMONIC, OPCODE, EXPR)                 : U-type instruction
 *     X_J(MNEMONIC, OPCODE, EXPR)                 : J-type instruction
 *
 * EXPR is the expression used in cpu_execute to execute the instruction
 * the following variables can be set (depending on the type of the instruction)
 *
 * cpu_t* cpu               : CPU state
 * const ins_t* instruction : decoded instruction
 * guest_reg* rd            : destination register
 * guest_reg* rs1           : first source register
 * guest_reg* rs2           : second source register
 * guest_word* rs1w         : first source register as a word
 * guest_word* rs2w         : second source register as a word
 * guest_reg_signed* rds    : destination register signed
 * guest_reg_signed* rs1s   : first source register signed
 * guest_reg_signed* rs2s   : second source register signed
 * guest_word_signed* rs1ws : first source register as a signed word
 * guest_word_signed* rs2ws : second source register as a signed word
 * int64_t imm              : immediate
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
	X_R(MUL, OPCODE_OP, F3_MUL, F7_MUL, *rd = *rs1 * *rs2)                                                                                             \
	X_R(MULH, OPCODE_OP, F3_MULH, F7_MULH, *rds = mulh(*rs1s, *rs2s))                                                                                  \
	X_R(MULHSU, OPCODE_OP, F3_MULHSU, F7_MULHSU, *rds = mulhsu(*rs1s, *rs2))                                                                           \
	X_R(MULHU, OPCODE_OP, F3_MULHU, F7_MULHU, *rd = mulhu(*rs1, *rs2))                                                                                 \
	X_R(DIV, OPCODE_OP, F3_DIV, F7_DIV, *rds = (*rs2s == 0) ? -1 : (*rs1s == INT64_MIN && *rs2s == -1) ? INT64_MIN                                     \
													   : (*rs1s / *rs2s))                              \
	X_R(DIVU, OPCODE_OP, F3_DIVU, F7_DIVU, *rd = (*rs2 == 0) ? UINT64_MAX : (*rs1 / *rs2))                                                             \
	X_R(REM, OPCODE_OP, F3_REM, F7_REM, *rds = (*rs2s == 0) ? *rs1s : (*rs1s == INT64_MIN && *rs2s == -1) ? 0                                          \
													      : (*rs1s % *rs2s))                           \
	X_R(REMU, OPCODE_OP, F3_REMU, F7_REMU, *rd = (*rs2 == 0) ? *rs1 : (*rs1 % *rs2))                                                                   \
                                                                                                                                                           \
	/* NOTE : We have to do the multiplication on 64 bits because overflowing a multiplication is undefined and LLVM can do */                         \
	/*        weird optimizations (see: <https://redd.it/18iaz89>) */                                                                                  \
	X_R(MULW, OPCODE_OP_32, F3_MUL, F7_MUL, *rds = (guest_word_signed)((guest_reg_signed)*rs1ws * *rs2ws))                                             \
	X_R(DIVW, OPCODE_OP_32, F3_DIV, F7_DIV, *rds = (*rs2ws == 0) ? -1 : (*rs1ws == INT32_MIN && *rs2ws == -1) ? INT32_MIN                              \
														  : (*rs1ws / *rs2ws))                     \
	X_R(DIVUW, OPCODE_OP_32, F3_DIVU, F7_DIVU, *rds = (guest_word_signed)((*rs2w == 0) ? UINT32_MAX : (*rs1w / *rs2w)))                                \
	X_R(REMW, OPCODE_OP_32, F3_REM, F7_REM, *rds = (*rs2ws == 0) ? *rs1ws : (*rs1ws == INT32_MIN && *rs2ws == -1) ? 0                                  \
														      : (*rs1ws % *rs2ws))                 \
	X_R(REMUW, OPCODE_OP_32, F3_REMU, F7_REMU, *rds = (guest_word_signed)((*rs2w == 0) ? *rs1w : (*rs1w % *rs2w)))                                     \
                                                                                                                                                           \
	/* NOTE : The instructions from the A extension are implemented "naively" as the emulator does everything in synchronized way */                   \
	/*        However this is more or less a violation of the spec as `SC` will always succeed even if it should fail */                               \
	/*        when no corresponding `LR` was executed */                                                                                               \
	X_R(LR_W, OPCODE_AMO, F3_AMO_W, F7_LR, *rds = (int32_t)emu_r32(emu, *rs1))                                                                         \
	X_R(                                                                                                                                               \
		SC_W, OPCODE_AMO, F3_AMO_W, F7_SC, do {                                                                                                    \
			emu_w32(emu, *rs1, *rs2w);                                                                                                         \
			*rd = 0;                                                                                                                           \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOSWAP_W, OPCODE_AMO, F3_AMO_W, F7_AMOSWAP, do {                                                                                          \
			int32_t value = (int32_t)emu_r32(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w32(emu, *rs1, *rs2w);                                                                                                 \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOADD_W, OPCODE_AMO, F3_AMO_W, F7_AMOADD, do {                                                                                            \
			int32_t value = (int32_t)emu_r32(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w32(emu, *rs1, *rs2ws + value);                                                                                        \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOXOR_W, OPCODE_AMO, F3_AMO_W, F7_AMOXOR, do {                                                                                            \
			int32_t value = (int32_t)emu_r32(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w32(emu, *rs1, *rs2w ^ value);                                                                                         \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOAND_W, OPCODE_AMO, F3_AMO_W, F7_AMOAND, do {                                                                                            \
			int32_t value = (int32_t)emu_r32(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w32(emu, *rs1, *rs2w& value);                                                                                          \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOOR_W, OPCODE_AMO, F3_AMO_W, F7_AMOOR, do {                                                                                              \
			int32_t value = (int32_t)emu_r32(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w32(emu, *rs1, *rs2w | value);                                                                                         \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOMIN_W, OPCODE_AMO, F3_AMO_W, F7_AMOMIN, do {                                                                                            \
			int32_t value = (int32_t)emu_r32(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w32(emu, *rs1, *rs2ws < value ? *rs2ws : value);                                                                       \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOMAX_W, OPCODE_AMO, F3_AMO_W, F7_AMOMAX, do {                                                                                            \
			int32_t value = (int32_t)emu_r32(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w32(emu, *rs1, *rs2ws > value ? *rs2ws : value);                                                                       \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOMINU_W, OPCODE_AMO, F3_AMO_W, F7_AMOMINU, do {                                                                                          \
			uint32_t value = emu_r32(emu, *rs1);                                                                                               \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w32(emu, *rs1, *rs2w < value ? *rs2w : value);                                                                         \
				*rds = (int32_t)value;                                                                                                     \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOMAXU_W, OPCODE_AMO, F3_AMO_W, F7_AMOMAXU, do {                                                                                          \
			uint32_t value = emu_r32(emu, *rs1);                                                                                               \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w32(emu, *rs1, *rs2w > value ? *rs2w : value);                                                                         \
				*rds = (int32_t)value;                                                                                                     \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
                                                                                                                                                           \
	X_R(LR_D, OPCODE_AMO, F3_AMO_D, F7_LR, *rd = emu_r64(emu, *rs1))                                                                                   \
	X_R(                                                                                                                                               \
		SC_D, OPCODE_AMO, F3_AMO_D, F7_SC, do {                                                                                                    \
			emu_w64(emu, *rs1, *rs2);                                                                                                          \
			*rd = 0;                                                                                                                           \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOSWAP_D, OPCODE_AMO, F3_AMO_D, F7_AMOSWAP, do {                                                                                          \
			int64_t value = (int64_t)emu_r64(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w64(emu, *rs1, *rs2);                                                                                                  \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOADD_D, OPCODE_AMO, F3_AMO_D, F7_AMOADD, do {                                                                                            \
			int64_t value = (int64_t)emu_r64(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w64(emu, *rs1, *rs2s + value);                                                                                         \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOXOR_D, OPCODE_AMO, F3_AMO_D, F7_AMOXOR, do {                                                                                            \
			int64_t value = (int64_t)emu_r64(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w64(emu, *rs1, *rs2 ^ value);                                                                                          \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOAND_D, OPCODE_AMO, F3_AMO_D, F7_AMOAND, do {                                                                                            \
			int64_t value = (int64_t)emu_r64(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w64(emu, *rs1, *rs2& value);                                                                                           \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOOR_D, OPCODE_AMO, F3_AMO_D, F7_AMOOR, do {                                                                                              \
			int64_t value = (int64_t)emu_r64(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w64(emu, *rs1, *rs2 | value);                                                                                          \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOMIN_D, OPCODE_AMO, F3_AMO_D, F7_AMOMIN, do {                                                                                            \
			int64_t value = (int64_t)emu_r64(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w64(emu, *rs1, *rs2s < value ? *rs2s : value);                                                                         \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOMAX_D, OPCODE_AMO, F3_AMO_D, F7_AMOMAX, do {                                                                                            \
			int64_t value = (int64_t)emu_r64(emu, *rs1);                                                                                       \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w64(emu, *rs1, *rs2s > value ? *rs2s : value);                                                                         \
				*rds = value;                                                                                                              \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOMINU_D, OPCODE_AMO, F3_AMO_D, F7_AMOMINU, do {                                                                                          \
			uint64_t value = emu_r64(emu, *rs1);                                                                                               \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w64(emu, *rs1, *rs2 < value ? *rs2 : value);                                                                           \
				*rd = value;                                                                                                               \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_R(                                                                                                                                               \
		AMOMAXU_D, OPCODE_AMO, F3_AMO_D, F7_AMOMAXU, do {                                                                                          \
			uint64_t value = emu_r64(emu, *rs1);                                                                                               \
			if (!cpu->exception_pending) {                                                                                                     \
				emu_w64(emu, *rs1, *rs2 > value ? *rs2 : value);                                                                           \
				*rd = value;                                                                                                               \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
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
	X_I(FENCEI, OPCODE_MISC_MEM, F3_FENCEI, /* nop */)                                                                                                 \
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
	X_I(                                                                                                                                               \
		CSRRW, OPCODE_SYSTEM, F3_CSRRW, do {                                                                                                       \
			if (rd == &cpu->regs[0]) {                                                                                                         \
				cpu_csr_write(emu, imm& CSR_MASK, *rs1);                                                                                   \
			} else {                                                                                                                           \
				*rd = cpu_csr_exchange(emu, imm & CSR_MASK, *rs1);                                                                         \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_I(                                                                                                                                               \
		CSRRS, OPCODE_SYSTEM, F3_CSRRS, do {                                                                                                       \
			if (rs1 == &cpu->regs[0]) {                                                                                                        \
				*rd = cpu_csr_read(emu, imm & CSR_MASK);                                                                                   \
			} else {                                                                                                                           \
				*rd = cpu_csr_set_bits(emu, imm & CSR_MASK, *rs1);                                                                         \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_I(                                                                                                                                               \
		CSRRC, OPCODE_SYSTEM, F3_CSRRC, do {                                                                                                       \
			if (rs1 == &cpu->regs[0]) {                                                                                                        \
				*rd = cpu_csr_read(emu, imm & CSR_MASK);                                                                                   \
			} else {                                                                                                                           \
				*rd = cpu_csr_clear_bits(emu, imm & CSR_MASK, *rs1);                                                                       \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
                                                                                                                                                           \
	X_I(                                                                                                                                               \
		CSRRWI, OPCODE_SYSTEM, F3_CSRRWI, do {                                                                                                     \
			if (rd == &cpu->regs[0]) {                                                                                                         \
				cpu_csr_write(emu, imm& CSR_MASK, (guest_reg)instruction->rs1);                                                            \
			} else {                                                                                                                           \
				*rd = cpu_csr_exchange(emu, imm & CSR_MASK, (guest_reg)instruction->rs1);                                                  \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_I(                                                                                                                                               \
		CSRRSI, OPCODE_SYSTEM, F3_CSRRSI, do {                                                                                                     \
			if (instruction->rs1 == 0) {                                                                                                       \
				*rd = cpu_csr_read(emu, imm & CSR_MASK);                                                                                   \
			} else {                                                                                                                           \
				*rd = cpu_csr_set_bits(emu, imm & CSR_MASK, instruction->rs1);                                                             \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
	X_I(                                                                                                                                               \
		CSRRCI, OPCODE_SYSTEM, F3_CSRRCI, do {                                                                                                     \
			if (instruction->rs1 == 0) {                                                                                                       \
				*rd = cpu_csr_read(emu, imm & CSR_MASK);                                                                                   \
			} else {                                                                                                                           \
				*rd = cpu_csr_clear_bits(emu, imm & CSR_MASK, instruction->rs1);                                                           \
			}                                                                                                                                  \
		} while (0))                                                                                                                               \
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

/* INS_TYPES : look-up-table from the opcode of the instruction to its type
 *             as the opcode is assumed to have its two LSBs set (only 4 bytes instructions are supported)
 *             the index in the table should be shifted 2 bits to the right
 */
extern const ins_type_t INS_TYPES[0x20];

#endif

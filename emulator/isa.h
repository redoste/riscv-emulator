#ifndef ISA_H
#define ISA_H

#include <stdint.h>

typedef uint64_t guest_paddr;
typedef uint64_t guest_reg;
typedef int64_t guest_reg_signed;

#define REG_COUNT 32
typedef uint8_t reg_t;

/* NOTE : we pack the enumeration to make sure ins_type_t values are encoded on one byte and
 *        prevent the INS_TYPES array to have 3/4 of its space wasted
 */
typedef enum __attribute__((packed)) ins_type_t {
	INS_TYPE_INVALID = 0,

	INS_TYPE_R,
	INS_TYPE_I,
	INS_TYPE_S,
	INS_TYPE_B,
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

#define DECODE_GET_OPCODE(insn) ((insn)&0x7f)
#define DECODE_GET_F3(insn)     (((insn) >> 12) & 0x7)
#define DECODE_GET_F7(insn)     (((insn) >> 25) & 0x7f)

#define DECODE_GET_RD(insn)  (((insn) >> 7) & 0x1f)
#define DECODE_GET_RS1(insn) (((insn) >> 15) & 0x1f)
#define DECODE_GET_RS2(insn) (((insn) >> 20) & 0x1f)

// NOTE : we cast to a signed integer the expression with the MSB to properly sign extend the value
// TODO : comment the fields with more details
#define DECODE_GET_I_IMM(insn) ((int32_t)(insn) >> 20)
#define DECODE_GET_S_IMM(insn)                                    \
	((int32_t)((((int32_t)(insn) >> (25 - 5)) & 0xffffffe0) | \
		   (((insn) >> 7) & 0x1f)))
#define DECODE_GET_B_IMM(insn)                                     \
	((int32_t)((((int32_t)(insn) >> (31 - 12)) & 0xfffff000) | \
		   (((insn) >> (25 - 5)) & 0x7e0) |                \
		   (((insn) >> (8 - 1)) & 0x1e) |                  \
		   ((((insn) >> 7) & 1) << 11)))
#define DECODE_GET_J_IMM(insn)                                     \
	((int32_t)((((int32_t)(insn) >> (31 - 20)) & 0xfff00000) | \
		   (((insn) >> (21 - 1)) & 0x7fe) |                \
		   ((((insn) >> 20) & 1) << 11) |                  \
		   ((insn)&0xff000)))

// TODO : make a common header for both the assembler and the emulator

/* RISC-V opcodes */
enum {
	OPCODE_OP = 0x33,
	OPCODE_OP_IMM = 0x13,
	OPCODE_LOAD = 0x03,
	OPCODE_STORE = 0x23,
	OPCODE_BRANCH = 0x63,
	OPCODE_JAL = 0x6f,
};

/* RISC-V funct3 */
enum {
	/* OPCODE OP */
	F3_ADD = 0,
	F3_SUB = 0,

	/* OPCODE OP_IMM */
	F3_ADDI = 0,
	F3_SLLI = 1,

	/* OPCODE LOAD */
	F3_LD = 3,

	/* OPCODE STORE */
	F3_SD = 3,

	/* OPCODE BRANCH */
	F3_BEQ = 0,
	F3_BNE = 1,
	F3_BLT = 4,
	F3_BGE = 5,
};

/* RISC-V funct7 */
enum {
	/* OPCODE OP / F3 ADD or F3 SUB */
	F7_ADD = 0x00,
	F7_SUB = 0x20,
};

/* X_INSTRUCTIONS : X-macro storing informations about all the instructions
 *                  the emulator can emulate
 *     X_R(MNEMONIC, OPCODE, FUNCT3, FUNCT7, EXPR) : R-type instruction
 *     X_I(MNEMONIC, OPCODE, FUNCT3, EXPR)         : I-type instruction
 *     X_S(MNEMONIC, OPCODE, FUNCT3, EXPR)         : S-type instruction
 *     X_B(MNEMONIC, OPCODE, FUNCT3, EXPR)         : B-type instruction
 *     X_J(MNEMONIC, OPCODE, EXPR)                 : J-type instruction
 */
#define X_INSTRUCTIONS                                                          \
	X_R(ADD, OPCODE_OP, F3_ADD, F7_ADD, *rd = *rs1 + *rs2)                  \
	X_R(SUB, OPCODE_OP, F3_SUB, F7_SUB, *rd = *rs1 - *rs2)                  \
	X_I(ADDI, OPCODE_OP_IMM, F3_ADDI, *rd = *rs1 + imm)                     \
	X_I(SLLI, OPCODE_OP_IMM, F3_SLLI, *rd = *rs1 << imm)                    \
	X_I(LD, OPCODE_LOAD, F3_LD, *rd = emu_r64(emu, *rs1 + imm))             \
	X_S(SD, OPCODE_STORE, F3_SD, emu_w64(emu, *rs1 + imm, *rs2))            \
	X_B(                                                                    \
		BEQ, OPCODE_BRANCH, F3_BEQ, do {                                \
			if (*rs1 == *rs2) {                                     \
				cpu->pc += imm;                                 \
				cpu->jump_pending = true;                       \
			}                                                       \
		} while (0))                                                    \
	X_B(                                                                    \
		BNE, OPCODE_BRANCH, F3_BNE, do {                                \
			if (*rs1 != *rs2) {                                     \
				cpu->pc += imm;                                 \
				cpu->jump_pending = true;                       \
			}                                                       \
		} while (0))                                                    \
	X_B(                                                                    \
		BLT, OPCODE_BRANCH, F3_BLT, do {                                \
			if ((guest_reg_signed)*rs1 < (guest_reg_signed)*rs2) {  \
				cpu->pc += imm;                                 \
				cpu->jump_pending = true;                       \
			}                                                       \
		} while (0))                                                    \
	X_B(                                                                    \
		BGE, OPCODE_BRANCH, F3_BGE, do {                                \
			if ((guest_reg_signed)*rs1 >= (guest_reg_signed)*rs2) { \
				cpu->pc += imm;                                 \
				cpu->jump_pending = true;                       \
			}                                                       \
		} while (0))                                                    \
	X_J(                                                                    \
		JAL, OPCODE_JAL, do {                                           \
			*rd = cpu->pc + 4;                                      \
			cpu->pc += imm;                                         \
			cpu->jump_pending = true;                               \
		} while (0))

extern ins_type_t INS_TYPES[0x20];

#endif

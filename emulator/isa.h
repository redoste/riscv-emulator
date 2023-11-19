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
#define DECODE_GET_U_IMM(insn) \
	((int32_t)((insn)&0xfffff000))
#define DECODE_GET_J_IMM(insn)                                     \
	((int32_t)((((int32_t)(insn) >> (31 - 20)) & 0xfff00000) | \
		   (((insn) >> (21 - 1)) & 0x7fe) |                \
		   ((((insn) >> 20) & 1) << 11) |                  \
		   ((insn)&0xff000)))

// TODO : make a common header for both the assembler and the emulator

/* RISC-V opcodes */
enum {
	OPCODE_AUIPC = 0x17,
	OPCODE_LUI = 0x37,

	OPCODE_OP = 0x33,
	OPCODE_OP_32 = 0x3b,
	OPCODE_OP_IMM = 0x13,
	OPCODE_OP_IMM_32 = 0x1b,

	OPCODE_MISC_MEM = 0x0F,
	OPCODE_SYSTEM = 0x73,

	OPCODE_LOAD = 0x03,
	OPCODE_STORE = 0x23,

	OPCODE_BRANCH = 0x63,
	OPCODE_JALR = 0x67,
	OPCODE_JAL = 0x6f,
};

/* RISC-V funct3 */
enum {
	/* OPCODE OP / OP-32 / OP-IMM / OP-IMM-32 */
	F3_ADD = 0,
	F3_SUB = 0,
	F3_SLL = 1,
	F3_SLT = 2,
	F3_SLTU = 3,
	F3_XOR = 4,
	F3_SRL = 5,
	F3_SRA = 5,
	F3_OR = 6,
	F3_AND = 7,

	/* OPCODE MISC-MEM */
	F3_FENCE = 0,
	F3_FENCE_I = 1,

	/* OPCODE SYSTEM */
	F3_ECALL = 0,
	F3_EBREAK = 0,
	F3_CSRRW = 1,
	F3_CSRRS = 2,
	F3_CSRRC = 3,
	F3_CSRRWI = 5,
	F3_CSRRSI = 6,
	F3_CSRRCI = 7,

	/* OPCODE LOAD */
	F3_LB = 0,
	F3_LH = 1,
	F3_LW = 2,
	F3_LD = 3,
	F3_LBU = 4,
	F3_LHU = 5,
	F3_LWU = 6,

	/* OPCODE STORE */
	F3_SB = 0,
	F3_SH = 1,
	F3_SW = 2,
	F3_SD = 3,

	/* OPCODE BRANCH */
	F3_BEQ = 0,
	F3_BNE = 1,
	F3_BLT = 4,
	F3_BGE = 5,
	F3_BLTU = 6,
	F3_BGEU = 7,

	/* OPCODE JALR */
	F3_JALR = 0,
};

/* RISC-V funct7 */
enum {
	/* OPCODE OP / OP-32 */
	F7_ADD = 0x00,
	F7_SUB = 0x20,
	F7_SLL = 0x00,
	F7_SLT = 0x00,
	F7_SLTU = 0x00,
	F7_XOR = 0x00,
	F7_SRL = 0x00,
	F7_SRA = 0x20,
	F7_OR = 0x00,
	F7_AND = 0x00,
};

/* X_INSTRUCTIONS : X-macro storing informations about all the instructions
 *                  the emulator can emulate
 *     X_R(MNEMONIC, OPCODE, FUNCT3, FUNCT7, EXPR) : R-type instruction
 *     X_I(MNEMONIC, OPCODE, FUNCT3, EXPR)         : I-type instruction
 *     X_S(MNEMONIC, OPCODE, FUNCT3, EXPR)         : S-type instruction
 *     X_B(MNEMONIC, OPCODE, FUNCT3, EXPR)         : B-type instruction
 *     X_U(MNEMONIC, OPCODE, EXPR)                 : U-type instruction
 *     X_J(MNEMONIC, OPCODE, EXPR)                 : J-type instruction
 */
#define X_INSTRUCTIONS                                                          \
	X_R(ADD, OPCODE_OP, F3_ADD, F7_ADD, *rd = *rs1 + *rs2)                  \
	X_R(SUB, OPCODE_OP, F3_SUB, F7_SUB, *rd = *rs1 - *rs2)                  \
	X_I(ADDI, OPCODE_OP_IMM, F3_ADD, *rd = *rs1 + imm)                      \
	X_I(SLLI, OPCODE_OP_IMM, F3_SLL, *rd = *rs1 << imm)                     \
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
	X_U(AUIPC, OPCODE_AUIPC, *rd = cpu->pc + imm)                           \
	X_U(LUI, OPCODE_LUI, *rd = imm)                                         \
	X_J(                                                                    \
		JAL, OPCODE_JAL, do {                                           \
			*rd = cpu->pc + 4;                                      \
			cpu->pc += imm;                                         \
			cpu->jump_pending = true;                               \
		} while (0))

extern ins_type_t INS_TYPES[0x20];

#endif

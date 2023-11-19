#ifndef ISA_H
#define ISA_H

#include <stdint.h>

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
 *                  the assembler can assemble
 *     X_R(MNEMONIC, OPCODE, FUNCT3, FUNCT7) : R-type instruction
 *     X_I(MNEMONIC, OPCODE, FUNCT3)         : I-type instruction
 *     X_I_S(MNEMONIC, OPCODE, FUNCT3)       : I-type instruction that is parsed like a S-type (i.e. `ld`)
 *     X_S(MNEMONIC, OPCODE, FUNCT3)         : S-type instruction
 *     X_B(MNEMONIC, OPCODE, FUNCT3)         : B-type instruction
 *     X_J(MNEMONIC, OPCODE)                 : J-type instruction
 *     X_P(MNEMONIC)                         : pseudo instruction
 */
#define X_INSTRUCTIONS                      \
	X_R(ADD, OPCODE_OP, F3_ADD, F7_ADD) \
	X_R(SUB, OPCODE_OP, F3_SUB, F7_SUB) \
	X_I(ADDI, OPCODE_OP_IMM, F3_ADD)    \
	X_I(SLLI, OPCODE_OP_IMM, F3_SLL)    \
	X_I_S(LD, OPCODE_LOAD, F3_LD)       \
	X_S(SD, OPCODE_STORE, F3_SD)        \
	X_B(BEQ, OPCODE_BRANCH, F3_BEQ)     \
	X_B(BNE, OPCODE_BRANCH, F3_BNE)     \
	X_B(BLT, OPCODE_BRANCH, F3_BLT)     \
	X_B(BGE, OPCODE_BRANCH, F3_BGE)     \
	X_J(JAL, OPCODE_JAL)                \
	X_P(J)                              \
	X_P(LI)                             \
	X_P(MV)

/* ins_mnemonic_t : enumeration of instruction mnemonics that the assembler
 *                  can parse
 */
typedef enum ins_mnemonic_t {
#define X_R(MNEMONIC, O, F3, F7) INS_##MNEMONIC,
#define X_I(MNEMONIC, O, F3)     INS_##MNEMONIC,
#define X_I_S(MNEMONIC, O, F3)   INS_##MNEMONIC,
#define X_S(MNEMONIC, O, F3)     INS_##MNEMONIC,
#define X_B(MNEMONIC, O, F3)     INS_##MNEMONIC,
#define X_J(MNEMONIC, O)         INS_##MNEMONIC,
#define X_P(MNEMONIC)            INS_##MNEMONIC,
	X_INSTRUCTIONS
#undef X_R
#undef X_I
#undef X_I_S
#undef X_S
#undef X_B
#undef X_J
#undef X_P

		INS_COUNT,
} ins_mnemonic_t;

/* REG_COUNT : maximum register number of RISC-V
 */
#define REG_COUNT 32

/* reg_t : typedef used to declare a value storing a register number
 */
typedef uint8_t reg_t;

/* INS_NAMES : string representation of the instruction mnemonics
 */
extern const char* const INS_NAMES[];

/* REG_ALIAS : string representation of the register aliases
 */
extern const char* const REG_ALIAS[];

/* ENCODE_x_INSTRUCTION : macros used to encode a single x-type instruction
 *                        the arguments of the macros are used multiple times thus an
 *                        expression with side-effects can have uninteded behaviour
 */
#define ENCODE_R_INSTRUCTION(opcode, f3, f7, rd, rs1, rs2)    \
	(((opcode)&0x7f) << 0) |       /* [0:6]   : opcode */ \
		(((rd)&0x1f) << 7) |   /* [7:11]  : rd */     \
		(((f3)&0x7) << 12) |   /* [12:14] : f3 */     \
		(((rs1)&0x1f) << 15) | /* [15:19] : rs1 */    \
		(((rs2)&0x1f) << 20) | /* [20:24] : rs2 */    \
		(((f7)&0x7f) << 25)    /* [25:31] : f7 */

#define ENCODE_I_INSTRUCTION(opcode, f3, rd, rs1, imm)        \
	(((opcode)&0x7f) << 0) |       /* [0:6]   : opcode */ \
		(((rd)&0x1f) << 7) |   /* [7:11]  : rd */     \
		(((f3)&0x7) << 12) |   /* [12:14] : f3 */     \
		(((rs1)&0x1f) << 15) | /* [15:19] : rs1 */    \
		(((imm)&0xfff) << 20)  /* [20:31] : imm[0:11] */

#define ENCODE_S_INSTRUCTION(opcode, f3, rs1, rs2, imm)                \
	(((opcode)&0x7f) << 0) |              /* [0:6]   : opcode */   \
		(((imm)&0x1f) << 7) |         /* [7:11]  : imm[0:4] */ \
		(((f3)&0x7) << 12) |          /* [12:14] : f3 */       \
		(((rs1)&0x1f) << 15) |        /* [15:19] : rs1 */      \
		(((rs2)&0x1f) << 20) |        /* [20:24] : rs2 */      \
		((((imm) >> 5) & 0x7f) << 25) /* [25:31] : imm[5:11] */

#define ENCODE_B_INSTRUCTION(opcode, f3, rs1, rs2, imm)                   \
	(((opcode)&0x7f) << 0) |                /* [0:6]   : opcode */    \
		((((imm) >> 11) & 0x1) << 7) |  /* [7]     : imm[11] */   \
		((((imm) >> 1) & 0xf) << 8) |   /* [8:11]  : imm[1:4] */  \
		(((f3)&0x7) << 12) |            /* [12:14] : f3 */        \
		(((rs1)&0x1f) << 15) |          /* [15:19] : rs1 */       \
		(((rs2)&0x1f) << 20) |          /* [20:24] : rs2 */       \
		((((imm) >> 5) & 0x3f) << 25) | /* [25:30] : imm[5:10] */ \
		((((imm) >> 12) & 0x1) << 31)   /* [31]    : imm[12] */

#define ENCODE_J_INSTRUCTION(opcode, rd, imm)                               \
	(((opcode)&0x7f) << 0) |                 /* [0:6]   : opcode */     \
		(((rd)&0x1f) << 7) |             /* [7:11]  : rd */         \
		((imm)&0xff000) |                /* [12:19] : imm[12:19] */ \
		((((imm) >> 11) & 0x1) << 20) |  /* [20]    : imm[11] */    \
		((((imm) >> 1) & 0x3ff) << 21) | /* [21:30] : imm[1:10] */  \
		((((imm) >> 20) & 0x1) << 31)    /* [31]    : imm[20] */

#endif

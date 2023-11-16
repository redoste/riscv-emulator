#ifndef ISA_H
#define ISA_H

#include <stdint.h>

/* RISC-V opcodes */
enum {
	OPCODE_ALU = 0x33,
	OPCODE_ALUI = 0x13,
	OPCODE_LOAD = 0x03,
	OPCODE_STORE = 0x23,
	OPCODE_BRANCH = 0x63,
	OPCODE_JAL = 0x6f,
};

/* RISC-V funct3 */
enum {
	/* OPCODE ALU */
	F3_ADD = 0,
	F3_SUB = 0,

	/* OPCODE ALUI */
	F3_ADDI = 0,

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
	/* OPCODE ALU / F3 ADD or F3 SUB */
	F7_ADD = 0x00,
	F7_SUB = 0x20,
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
#define X_INSTRUCTIONS                       \
	X_R(ADD, OPCODE_ALU, F3_ADD, F7_ADD) \
	X_R(SUB, OPCODE_ALU, F3_SUB, F7_SUB) \
	X_I(ADDI, OPCODE_ALUI, F3_ADDI)      \
	X_I_S(LD, OPCODE_LOAD, F3_LD)        \
	X_S(SD, OPCODE_STORE, F3_SD)         \
	X_B(BEQ, OPCODE_BRANCH, F3_BEQ)      \
	X_B(BNE, OPCODE_BRANCH, F3_BNE)      \
	X_B(BLT, OPCODE_BRANCH, F3_BLT)      \
	X_B(BGE, OPCODE_BRANCH, F3_BGE)      \
	X_J(JAL, OPCODE_JAL)                 \
	X_P(J)                               \
	X_P(LI)                              \
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

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
 *     X_I_S(MNEMONIC, OPCODE, FUNCT3)       : I-type instruction that is parsed like a S-type (e.g. loads)
 *     X_S(MNEMONIC, OPCODE, FUNCT3)         : S-type instruction
 *     X_B(MNEMONIC, OPCODE, FUNCT3)         : B-type instruction
 *     X_U(MNEMONIC, OPCODE)                 : U-type instruction
 *     X_J(MNEMONIC, OPCODE)                 : J-type instruction
 *     X_P(MNEMONIC)                         : pseudo instruction
 */
#define X_INSTRUCTIONS                                \
	X_R(ADD, OPCODE_OP, F3_ADD, F7_ADD)           \
	X_R(SUB, OPCODE_OP, F3_SUB, F7_SUB)           \
	X_R(SLL, OPCODE_OP, F3_SLL, F7_SLL)           \
	X_R(SLT, OPCODE_OP, F3_SLT, F7_SLT)           \
	X_R(SLTU, OPCODE_OP, F3_SLTU, F7_SLTU)        \
	X_R(XOR, OPCODE_OP, F3_XOR, F7_XOR)           \
	X_R(SRL, OPCODE_OP, F3_SRL, F7_SRL)           \
	X_R(SRA, OPCODE_OP, F3_SRA, F7_SRA)           \
	X_R(OR, OPCODE_OP, F3_OR, F7_OR)              \
	X_R(AND, OPCODE_OP, F3_AND, F7_AND)           \
                                                      \
	X_R(ADDW, OPCODE_OP_32, F3_ADD, F7_ADD)       \
	X_R(SUBW, OPCODE_OP_32, F3_SUB, F7_SUB)       \
	X_R(SLLW, OPCODE_OP_32, F3_SLL, F7_SLL)       \
	X_R(SRLW, OPCODE_OP_32, F3_SRL, F7_SRL)       \
	X_R(SRAW, OPCODE_OP_32, F3_SRA, F7_SRA)       \
                                                      \
	X_I_S(LB, OPCODE_LOAD, F3_LB)                 \
	X_I_S(LH, OPCODE_LOAD, F3_LH)                 \
	X_I_S(LW, OPCODE_LOAD, F3_LW)                 \
	X_I_S(LD, OPCODE_LOAD, F3_LD)                 \
	X_I_S(LBU, OPCODE_LOAD, F3_LBU)               \
	X_I_S(LHU, OPCODE_LOAD, F3_LHU)               \
	X_I_S(LWU, OPCODE_LOAD, F3_LWU)               \
                                                      \
	/* TODO : parse properly fence and fence.i */ \
	/* e.g. : 0f 00 30 0d  fence   iow, rw */     \
	/*        0f 10 00 00  fence.i */             \
	X_I(FENCE, OPCODE_MISC_MEM, F3_FENCE)         \
	X_I(FENCE_I, OPCODE_MISC_MEM, F3_FENCE_I)     \
                                                      \
	X_I(ADDI, OPCODE_OP_IMM, F3_ADD)              \
	X_I(SLLI, OPCODE_OP_IMM, F3_SLL)              \
	X_I(SLTI, OPCODE_OP_IMM, F3_SLT)              \
	X_I(SLTIU, OPCODE_OP_IMM, F3_SLTU)            \
	X_I(XORI, OPCODE_OP_IMM, F3_XOR)              \
	/* TODO : add SRAI */                         \
	X_I(SRLI, OPCODE_OP_IMM, F3_SRL)              \
	X_I(ORI, OPCODE_OP_IMM, F3_OR)                \
	X_I(ANDI, OPCODE_OP_IMM, F3_AND)              \
                                                      \
	X_I(ADDIW, OPCODE_OP_IMM_32, F3_ADD)          \
	/* TODO : add SRAIW */                        \
	X_I(SLLIW, OPCODE_OP_IMM_32, F3_SLL)          \
	X_I(SRLIW, OPCODE_OP_IMM_32, F3_SRL)          \
                                                      \
	X_I_S(JALR, OPCODE_JALR, F3_JALR)             \
                                                      \
	/* TODO : support ecall & ebreak */           \
	X_I(ECALL, OPCODE_SYSTEM, F3_ECALL)           \
	/* TODO : support CSR instructions */         \
	/* X_I(CSRRW, OPCODE_SYSTEM, F3_CSRRW)   */   \
	/* X_I(CSRRS, OPCODE_SYSTEM, F3_CSRRS)   */   \
	/* X_I(CSRRC, OPCODE_SYSTEM, F3_CSRRC)   */   \
	/* X_I(CSRRWI, OPCODE_SYSTEM, F3_CSRRWI) */   \
	/* X_I(CSRRSI, OPCODE_SYSTEM, F3_CSRRSI) */   \
	/* X_I(CSRRCI, OPCODE_SYSTEM, F3_CSRRCI) */   \
                                                      \
	X_S(SB, OPCODE_STORE, F3_SB)                  \
	X_S(SH, OPCODE_STORE, F3_SH)                  \
	X_S(SW, OPCODE_STORE, F3_SW)                  \
	X_S(SD, OPCODE_STORE, F3_SD)                  \
	X_B(BEQ, OPCODE_BRANCH, F3_BEQ)               \
	X_B(BNE, OPCODE_BRANCH, F3_BNE)               \
	X_B(BLT, OPCODE_BRANCH, F3_BLT)               \
	X_B(BGE, OPCODE_BRANCH, F3_BGE)               \
	X_B(BLTU, OPCODE_BRANCH, F3_BLTU)             \
	X_B(BGEU, OPCODE_BRANCH, F3_BGEU)             \
                                                      \
	X_U(AUIPC, OPCODE_AUIPC)                      \
	X_U(LUI, OPCODE_LUI)                          \
                                                      \
	X_J(JAL, OPCODE_JAL)                          \
                                                      \
	X_P(J)                                        \
	X_P(LI)                                       \
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
#define X_U(MNEMONIC, O)         INS_##MNEMONIC,
#define X_J(MNEMONIC, O)         INS_##MNEMONIC,
#define X_P(MNEMONIC)            INS_##MNEMONIC,
	X_INSTRUCTIONS
#undef X_R
#undef X_I
#undef X_I_S
#undef X_S
#undef X_B
#undef X_U
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

#define ENCODE_U_INSTRUCTION(opcode, rd, imm)               \
	(((opcode)&0x7f) << 0) |     /* [0:6]   : opcode */ \
		(((rd)&0x1f) << 7) | /* [7:11]  : rd */     \
		((imm)&0xfffff000)   /* [12:31] : imm[12:31] */

#define ENCODE_J_INSTRUCTION(opcode, rd, imm)                               \
	(((opcode)&0x7f) << 0) |                 /* [0:6]   : opcode */     \
		(((rd)&0x1f) << 7) |             /* [7:11]  : rd */         \
		((imm)&0xff000) |                /* [12:19] : imm[12:19] */ \
		((((imm) >> 11) & 0x1) << 20) |  /* [20]    : imm[11] */    \
		((((imm) >> 1) & 0x3ff) << 21) | /* [21:30] : imm[1:10] */  \
		((((imm) >> 20) & 0x1) << 31)    /* [31]    : imm[20] */

#endif

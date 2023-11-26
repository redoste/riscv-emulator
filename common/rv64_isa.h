#ifndef RV64_ISA_H
#define RV64_ISA_H

#include <stdint.h>

/* NOTE : while this header is common to both the assembler and the emulator
 *        the main X_INSTRUCTIONS macro is still separated
 *        we need to find a clean way to make it common while keeping it usable
 *        for both party (i.e. without a lot of unused parameters for one of the
 *        party)
 */

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

	/* OPCODE SYSTEM */
	F3_ECALL = 0,
	F3_EBREAK = 0,

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

/* RISC-V funct12 */
enum {
	/* OPCODE OP-IMM / OP-IMM-32 */
	F12_SRA = 0x400,

	/* OPCODE SYSTEM */
	F12_ECALL = 0,
	F12_EBREAK = 1,
};

/* RISC-V FENCE operand bits */
enum {
	FENCE_OPERAND_I = (1 << 3),
	FENCE_OPERAND_O = (1 << 2),
	FENCE_OPERAND_R = (1 << 1),
	FENCE_OPERAND_W = (1 << 0),
};

/* DECODE_GET_x : macros to get a specific field from an encoded instruction
 *                it's up to the caller to make sure the encoded instruction type has the asked
 *                field
 */
#define DECODE_GET_OPCODE(insn) ((insn)&0x7f)
#define DECODE_GET_F3(insn)     (((insn) >> 12) & 0x7)
#define DECODE_GET_F7(insn)     (((insn) >> 25) & 0x7f)

#define DECODE_GET_RD(insn)  (((insn) >> 7) & 0x1f)
#define DECODE_GET_RS1(insn) (((insn) >> 15) & 0x1f)
#define DECODE_GET_RS2(insn) (((insn) >> 20) & 0x1f)

/* DECODE_GET_x_IMM : macros to get the immediate from a x-type encoded instruction
 *                    the argument of the macros is used multiple times thus an
 *                    expression with side-effects can have unintended behaviour
 * NOTE : we cast to a signed integer the expression with the MSB to properly sign extend the value
 *        and recast a second time on the parent expression to make sure the result is treated as
 *        signed and will be sign extended regardless of the type of the lvalue
 */
#define DECODE_GET_I_IMM(insn) \
	((int32_t)(insn) >> 20) /* [11:31] : insn[31] */
				/* [0:10]  : insn[20:30] */

#define DECODE_GET_S_IMM(insn)                                                                \
	((int32_t)((((int32_t)(insn) >> (25 - 5)) & 0xffffffe0) | /* [11:31] : insn[31]       \
								     [5:10]  : insn[25:30] */ \
		   (((insn) >> 7) & 0x1f)))                       /* [0:4]   : insn[7:11] */

#define DECODE_GET_B_IMM(insn)                                                                 \
	((int32_t)((((int32_t)(insn) >> (31 - 12)) & 0xfffff000) | /* [12:31] : insn[31] */    \
		   ((((insn) >> 7) & 1) << 11) |                   /* [11]    : insn[7] */     \
		   (((insn) >> (25 - 5)) & 0x7e0) |                /* [5:10]  : insn[25:30] */ \
		   (((insn) >> (8 - 1)) & 0x1e)))                  /* [1:4]   : insn[8:11] */

#define DECODE_GET_U_IMM(insn) \
	((int32_t)((insn)&0xfffff000)) /* [12:31] : insn[12:13] */

#define DECODE_GET_J_IMM(insn)                                                                 \
	((int32_t)((((int32_t)(insn) >> (31 - 20)) & 0xfff00000) | /* [20:31] : insn[31] */    \
		   ((insn)&0xff000) |                              /* [12:19] : insn[12:19] */ \
		   ((((insn) >> 20) & 1) << 11) |                  /* [11]    : insn[20] */    \
		   (((insn) >> (21 - 1)) & 0x7fe)))                /* [1:10]  : insn[21:30] */

/* ENCODE_x_INSTRUCTION : macros used to encode a single x-type instruction
 *                        the arguments of the macros are used multiple times thus an
 *                        expression with side-effects can have unintended behaviour
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

/* guest_paddr : typedef used to declare a value storing a guest physical address
 */
typedef uint64_t guest_paddr;

/* guest_reg : typedef used to declare a value storing an unsigned guest register
 */
typedef uint64_t guest_reg;

/* guest_reg_signed : typedef used to declare a value storing a signed guest register
 */
typedef int64_t guest_reg_signed;

/* guest_word : typedef used to declare a value storing an unsigned guest word
 */
typedef uint32_t guest_word;

/* guest_word_signed : typedef used to declare a value storing a signed guest word
 */
typedef int32_t guest_word_signed;

/* REG_COUNT : maximum register number of RISC-V
 */
#define REG_COUNT 32

/* reg_t : typedef used to declare a value storing a register number
 */
typedef uint8_t reg_t;

#endif

#ifndef X86_ISA_H
#define X86_ISA_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* x86_reg_t : enumeration of the x86-64 registers
 */
typedef enum x86_reg_t {
	RAX = 0x0,
	RCX = 0x1,
	RDX = 0x2,
	RBX = 0x3,
	RSP = 0x4,
	RBP = 0x5,
	RSI = 0x6,
	RDI = 0x7,
	R8 = 0x8,
	R9 = 0x9,
	R10 = 0xa,
	R11 = 0xb,
	R12 = 0xc,
	R13 = 0xd,
	R14 = 0xe,
	R15 = 0xf,
} x86_reg_t;

/* x86_addr_mode_t : addressing modes of a x86-64 instruction with or without a
 *                   R/M byte
 */
typedef enum x86_addr_mode_t {
	X86_AM_REG,               // reg
	X86_AM_RM,                // r/m
	X86_AM_DEREF_RM,          // [r/m]
	X86_AM_DEREF_RM_DISP_8,   // [r/m + disp8]
	X86_AM_DEREF_RM_DISP_32,  // [r/m + disp32]
} x86_addr_mode_t;

/* x86_ins_t : structure representing a x86-64 instruction that was already
 *             assembled but not encoded
 */
typedef struct x86_ins_t {
	uint64_t opcode;
	size_t opcode_size;
	x86_addr_mode_t addr_mode;
	x86_reg_t reg;
	x86_reg_t rm;
	int64_t disp;
	int64_t imm;
	size_t imm_size;
	bool rex_w;
} x86_ins_t;

/* x86_reloc_type_t : enumeration of the kind of relocations that can be emitted
 *                    by the instruction encoder
 */
typedef enum x86_reloc_type_t {
	X86_RELOC_NONE = 0,
	X86_RELOC_IMM,
	X86_RELOC_DISP,
} x86_reloc_type_t;

/* x86_operand_type_t : enumeration of the kind of operands the assembler can
 *                      interpret
 */
typedef enum x86_operand_type_t {
	X86_OPERAND_NONE = 0,  // none
	X86_OPERAND_IMM,       // immediate
	X86_OPERAND_REG,       // register
	X86_OPERAND_DEREF,     // [reg]
	X86_OPERAND_DISP,      // [reg + disp]

	X86_OPERAND_RELOC_DISP8,  // relocated [reg + disp]
	X86_OPERAND_RELOC_IMM32,  // relocated immedaite
} x86_operand_type_t;

/* x86_operand_t : typedef used to declare a x86-64 operand
 *                 we build a bitfield in the following format :
 *     bit  0 - bit  3 : register number
 *     bit  8 - bit 39 : disp
 *  or
 *     bit  0 - bit 31 : immediate
 *
 *     bit 56 - bit 63 : operand type
 */
typedef uint64_t x86_operand_t;

/* OP_X : macros used to express a x86_operand_t of the kind X
 */
#define OP_IMM(i) (((x86_operand_t)X86_OPERAND_IMM << 56) | \
		   ((i)&0xffffffff))
#define OP_REG(r) (((x86_operand_t)X86_OPERAND_REG << 56) | \
		   ((r)&0xf))
#define OP_DEREF(r) (((x86_operand_t)X86_OPERAND_DEREF << 56) | \
		     ((r)&0xf))
#define OP_DISP(r, d) (((x86_operand_t)X86_OPERAND_DISP << 56) | \
		       ((uint64_t)((d)&0xffffffff) << 8) |       \
		       ((r)&0xf))

#define OP_RELOC_IMM32    ((x86_operand_t)X86_OPERAND_RELOC_IMM32 << 56)
#define OP_RELOC_DISP8(r) (((x86_operand_t)X86_OPERAND_RELOC_DISP8 << 56) | \
			   ((r)&0xf))

/* OP_GET_X : macros used to get the field X from a x86_operand_t
 */
#define OP_GET_TYPE(x) ((x) >> 56)
#define OP_GET_IMM(x)  ((int32_t)((x)&0xffffffff))
#define OP_GET_REG(x)  ((x)&0xf)
#define OP_GET_DISP(x) ((int32_t)(((x) >> 8) & 0xffffffff))

/* X_X86_MNEMONICS : X-macro of all the mnemonics supported by the x86-64
 *                   assembler
 */
#define X_X86_MNEMONICS \
	X(MOV)          \
	X(MOVSX)        \
	X(MOVZX)        \
	X(ADD)          \
	X(SUB)          \
	X(NEG)          \
	X(SHL)          \
	X(SHR)          \
	X(SAR)          \
	X(XOR)          \
	X(OR)           \
	X(AND)          \
	X(CMP)          \
	X(TEST)         \
	X(SETL)         \
	X(SETB)         \
	X(MUL)          \
	X(IMUL)         \
	X(CQO)          \
	X(DIV)          \
	X(IDIV)         \
	X(JZ)           \
	X(JNZ)          \
	X(JL)           \
	X(JGE)          \
	X(JNC)          \
	X(JC)           \
	X(JMP)

/* x86_mnemonic_t : enumeration of the supported x86-64 mnemonics
 */
typedef enum x86_mnemonic_t {
#define X(MNEMONIC) X86_MNEMONIC_##MNEMONIC,
	X_X86_MNEMONICS
#undef X

		X86_MNEMONIC_COUNT,
} x86_mnemonic_t;

/* x86_ins_encoding_operand_type_t : enumeration of the kind of operands the instruction
 *                                   encoder can encode
 */
typedef enum x86_ins_encoding_operand_type_t {
	X86_OPERAND_ENCODING_NONE = 0,
	X86_OPERAND_ENCODING_RM64,
	X86_OPERAND_ENCODING_R64,
	X86_OPERAND_ENCODING_IMM8,
	X86_OPERAND_ENCODING_IMM32,
	X86_OPERAND_ENCODING_IMM64,
} x86_ins_encoding_operand_type_t;

/* x86_ins_encoding_t : structure storing informations about the encoding of a x86-64
 *                      instruction
 */
typedef struct x86_ins_encoding_t {
	uint64_t opcode;
	size_t opcode_size;
	uint8_t rm_filler;
	bool rex_w;
	x86_ins_encoding_operand_type_t dst;
	x86_ins_encoding_operand_type_t src;
} x86_ins_encoding_t;

/* X86_INS_ENCODING : list of the instruction encodings supported, indexed the mnemonic
 */
extern const x86_ins_encoding_t* const X86_INS_ENCODINGS[];

#endif

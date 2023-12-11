#ifndef CODEGEN_H
#define CODEGEN_H

#include <stdbool.h>

#include "x86_isa.h"

/* codegen_start_ins : emit the start of the array for a RISC-V instruction
 *     const char* mnemonic : mnemonic of the instruction
 */
void codegen_start_ins(const char* mnemonic);

/* codegen_end_ins : emit the end of the array for a RISC-V instruction
 */
void codegen_end_ins(void);

/* codegen_reloc_type_t : enumeration of the kind of relocations that can be
 *                        applied on the pre-assembled x86-64 code
 */
typedef enum codegen_reloc_type_t {
	CODEGEN_RELOC_NONE = 0,
	CODEGEN_RELOC_RS1,
	CODEGEN_RELOC_RS2,
	CODEGEN_RELOC_RD,
	CODEGEN_RELOC_IMM,
} codegen_reloc_type_t;

/* codegen_start_line : start an entry in the dr_x86_code_t array
 *     bool bX : bits identifying the specific implementation
 *               for now they are used to implement differently when a register is x0
 */
void codegen_start_line(bool b0, bool b1, bool b2);

/* codegen_asm : emit a single x86-64 instruction
 *     x86_mnemonic_t mnemonic         : mnemonic of the instruction to emit
 *     x86_operand_t dst               : destination operand
 *     x86_operand_t src               : source operand
 *     codegen_reloc_type_t reloc_type : if a relocation is present, its kind
 */
void codegen_asm(x86_mnemonic_t mnemonic, x86_operand_t dst, x86_operand_t src, codegen_reloc_type_t reloc_type);

/* codegen_end_line : end the entry in the dr_x86_code_t array
 */
void codegen_end_line(void);

/* C_X : macros used to declare the emitting function of a X-type instruction
 */
#define C_R(MNEMONIC) static inline void codegen_##MNEMONIC(bool rs1_zero, bool rs2_zero, bool rd_zero)
#define C_I(MNEMONIC) static inline void codegen_##MNEMONIC(bool rs1_zero, bool rd_zero)
#define C_S(MNEMONIC) static inline void codegen_##MNEMONIC(bool rs1_zero, bool rs2_zero)
#define C_B(MNEMONIC) static inline void codegen_##MNEMONIC(bool rs1_zero, bool rs2_zero)
#define C_U(MNEMONIC) static inline void codegen_##MNEMONIC(bool rd_zero)
#define C_J(MNEMONIC) static inline void codegen_##MNEMONIC(bool rd_zero)

/* S_X : macros used to start the line of a X-type instruction
 */
#define S_R() codegen_start_line(rs1_zero, rs2_zero, rd_zero)
#define S_I() codegen_start_line(rs1_zero, rd_zero, 0)
#define S_S() codegen_start_line(rs1_zero, rs2_zero, 0)
#define S_B() codegen_start_line(rs1_zero, rs2_zero, 0)
#define S_U() codegen_start_line(rd_zero, 0, 0)
#define S_J() codegen_start_line(rd_zero, 0, 0)

/* A_X : macros used to emit a x86-64 instruction with a X relocation
 */
#define A(MNEMONIC, DST, SRC)     codegen_asm(X86_MNEMONIC_##MNEMONIC, DST, SRC, CODEGEN_RELOC_NONE)
#define A_RS1(MNEMONIC, DST, SRC) codegen_asm(X86_MNEMONIC_##MNEMONIC, DST, SRC, CODEGEN_RELOC_RS1)
#define A_RS2(MNEMONIC, DST, SRC) codegen_asm(X86_MNEMONIC_##MNEMONIC, DST, SRC, CODEGEN_RELOC_RS2)
#define A_RD(MNEMONIC, DST, SRC)  codegen_asm(X86_MNEMONIC_##MNEMONIC, DST, SRC, CODEGEN_RELOC_RD)
#define A_IMM(MNEMONIC, DST, SRC) codegen_asm(X86_MNEMONIC_##MNEMONIC, DST, SRC, CODEGEN_RELOC_IMM)

/* OP_RELOC_RV_REG : macro used to easily express a DISP8 relocation relative to
 *                   the x86-64 register holding the base of the RISC-V registers
 */
#define OP_RELOC_RV_REG OP_RELOC_DISP8(R8)

/* EMU_FUNCTION : macro used to do a function call to an emulator function in the
 *                `dr_emu_functions` table pointed by R11
 */
#define EMU_FUNCTION(index)                                                             \
	do {                                                                            \
		A(PUSH, OP_REG(R8), 0);                                                 \
		A(PUSH, OP_REG(R9), 0);                                                 \
		A(PUSH, OP_REG(R10), 0);                                                \
		A(PUSH, OP_REG(R11), 0);                                                \
		/* TODO : We should probably save R9 back to the PC field of `cpu_t` */ \
		/*        in case one of the emu function needs the current PC */       \
		A(MOV, OP_REG(RDI), OP_REG(R12));                                       \
		A(CALL, OP_DISP(R11, index * 8), 0);                                    \
		A(POP, OP_REG(R11), 0);                                                 \
		A(POP, OP_REG(R10), 0);                                                 \
		A(POP, OP_REG(R9), 0);                                                  \
		A(POP, OP_REG(R8), 0);                                                  \
	} while (0)

/* E : macro used to end the line of an instruction and increment PC
 */
#define E()                                    \
	do {                                   \
		A(ADD, OP_REG(R9), OP_IMM(4)); \
		codegen_end_line();            \
		return;                        \
	} while (0)

/* E_J : macro used to end the line of an instruction and call `dr_exit` with a new
 *       updated PC
 */
#define E_J()                           \
	do {                            \
		A(JMP, OP_REG(R10), 0); \
		codegen_end_line();     \
		return;                 \
	} while (0)

#endif

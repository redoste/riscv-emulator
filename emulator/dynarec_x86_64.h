#ifndef DYNAREC_X86_64_H
#define DYNAREC_X86_64_H

#ifdef RISCV_EMULATOR_DYNAREC_X86_64_SUPPORT

#ifndef __x86_64__
#pragma GCC error "dynarec is only supported on x86-64"
#endif

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include "isa.h"

/* dr_x86_code_t : structure storing informations about some pre-assembled x86-64
 *                 code matching a RISC-V instruction
 */
typedef struct dr_x86_code_t {
	uint8_t* code;
	size_t code_size;
	ssize_t rs1_reloc;
	ssize_t rs2_reloc;
	ssize_t rd_reloc;
	ssize_t imm_reloc;
	ssize_t rs1uimm_reloc;
} dr_x86_code_t;

/* DYNAREC_PAGE_SIZE : size of a single page of x86-64 code, here we assume pages are 4KiB
 *                     (and on x86-64 it's always the case)
 */
#define DYNAREC_PAGE_SIZE 0x1000

/* DYNAREC_PAGE_MASK : mask used to get the base of a page of x86-64 code
 */
#define DYNAREC_PAGE_MASK ~0xfffll

/* DYNAREC_PROLOGUE_SIZE : size of the prologue code that the function emitting code will
 *                         guarantee to keep available
 */
#define DYNAREC_PROLOGUE_SIZE 3

/* dr_block_t : structure storing informations about a block of code being emitted
 */
typedef struct dr_block_t {
	uint8_t* page;
	size_t pos;
	guest_vaddr base;
	guest_vaddr pc;
} dr_block_t;

/* dr_ins_t : structure storing informations about a recompiled instruction in the
 *            instruction cache
 */
typedef struct dr_ins_t {
	guest_vaddr tag;
	guest_vaddr block_entry;
	uint8_t* native_code;
} dr_ins_t;

// NOTE : forward declaration to deal with a cyclic dependency with emulator.h
typedef struct emulator_t emulator_t;

/* dr_emit_block : emit a block of x86-64 code
 *                 returns true if some x86-64 code was added to the instruction cache
 *                 returns false otherwise
 *     emulator_t* emu  : pointer to the emulator
 *     guest_vaddr base : base RISC-V program counter of the block to emit
 */
bool dr_emit_block(emulator_t* emu, guest_vaddr base);

/* dr_free : free all the allocated pages still used by the instruction cache
 *     emulator_t* emu : pointer to the emulator
 */
void dr_free(emulator_t* emu);

/* dr_entry : enter dynarec code
 *            returns the new RISC-V program counter
 *     emulator_t* emu   : pointer to the emulator
 *     void* native_code : pointer to the emitted native code
 *     guest_reg* regs   : pointer to the RISC-V register array
 *     guest_reg pc      : current PC
 */
guest_reg dr_entry(emulator_t* emu, void* native_code, guest_reg* regs, guest_reg pc);

/* DR_X86_X : dr_x86_code_t corresponding to the RISC-V instruction X
 *            these arrays are generated by the code generator in dynarec_x86_64_codegen/
 */
#define X(MNEMONIC)                                    extern const dr_x86_code_t DR_X86_##MNEMONIC[];
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)            X(MNEMONIC)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)                X(MNEMONIC)
#define X_I_IMM(MNEMONIC, OPCODE, F3, EXPR, F12S, F7S) X(MNEMONIC)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)                X(MNEMONIC)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)                X(MNEMONIC)
#define X_U(MNEMONIC, OPCODE, EXPR)                    X(MNEMONIC)
#define X_J(MNEMONIC, OPCODE, EXPR)                    X(MNEMONIC)
X_INSTRUCTIONS
#undef X
#undef X_R
#undef X_I
#undef X_I_IMM
#undef X_S
#undef X_B
#undef X_U
#undef X_J

#endif

#endif

#include <assert.h>
#include <endian.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "dynarec_x86_64.h"
#include "emulator.h"
#include "isa.h"

#ifdef __BYTE_ORDER
#if __BYTE_ORDER == __LITTLE_ENDIAN
#define REG_WORD_OFFSET 0
#else
#define REG_WORD_OFFSET 4
#endif
#else
#pragma GCC warning "__BYTE_ORDER undefined, assuming little endian"
#define REG_WORD_OFFSET 0
#endif

#ifdef __SIZEOF_INT128__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpedantic"
// We ignore -Wpedantic because __int128 isn't part of the ISO C standard

static inline guest_reg_signed mulh(guest_reg_signed rs1, guest_reg_signed rs2) {
	return ((__int128)rs1 * (__int128)rs2) >> 64;
}

static inline guest_reg_signed mulhsu(guest_reg_signed rs1, guest_reg rs2) {
	return ((__int128)rs1 * (unsigned __int128)rs2) >> 64;
}

static inline guest_reg mulhu(guest_reg rs1, guest_reg rs2) {
	return ((unsigned __int128)rs1 * (unsigned __int128)rs2) >> 64;
}

#pragma GCC diagnostic pop
#else
static inline guest_reg_signed mulh(guest_reg_signed rs1, guest_reg_signed rs2) {
	int64_t rs1l = rs1 & 0xffffffff, rs1h = rs1 >> 32;
	int64_t rs2l = rs2 & 0xffffffff, rs2h = rs2 >> 32;

	int64_t m0 = rs1l * rs2l;
	int64_t m1 = rs1h * rs2l;
	int64_t m2 = rs1l * rs2h;
	int64_t m3 = rs1h * rs2h;

	m3 += m2 >> 32;
	m1 += (m0 >> 32) + (m2 & 0xffffffff);

	return m3 + (m1 >> 32);
}

static inline guest_reg_signed mulhsu(guest_reg_signed rs1, guest_reg rs2) {
	int64_t rs1l = rs1 & 0xffffffff, rs1h = rs1 >> 32;
	uint64_t rs2l = rs2 & 0xffffffff, rs2h = rs2 >> 32;

	uint64_t m0 = rs1l * rs2l;
	int64_t m1 = rs1h * rs2l;
	uint64_t m2 = rs1l * rs2h;
	int64_t m3 = rs1h * rs2h;

	m3 += m2 >> 32;
	m1 += (m0 >> 32) + (m2 & 0xffffffff);

	return m3 + (m1 >> 32);
}

static inline guest_reg mulhu(guest_reg rs1, guest_reg rs2) {
	uint64_t rs1l = rs1 & 0xffffffff, rs1h = rs1 >> 32;
	uint64_t rs2l = rs2 & 0xffffffff, rs2h = rs2 >> 32;

	uint64_t m0 = rs1l * rs2l;
	uint64_t m1 = rs1h * rs2l;
	uint64_t m2 = rs1l * rs2h;
	uint64_t m3 = rs1h * rs2h;

	m3 += m2 >> 32;
	m1 += (m0 >> 32) + (m2 & 0xffffffff);

	return m3 + (m1 >> 32);
}
#endif

static void cpu_execute_type_r(emulator_t* emu, const ins_t* instruction) {
	cpu_t* cpu = &emu->cpu;

	guest_reg* rd = &cpu->regs[instruction->rd];
	guest_reg* rs1 = &cpu->regs[instruction->rs1];
	guest_reg* rs2 = &cpu->regs[instruction->rs2];
	guest_word* rs1w = (guest_word*)((uint8_t*)&cpu->regs[instruction->rs1] + REG_WORD_OFFSET);
	guest_word* rs2w = (guest_word*)((uint8_t*)&cpu->regs[instruction->rs2] + REG_WORD_OFFSET);

	guest_reg_signed* rds = (guest_reg_signed*)&cpu->regs[instruction->rd];
	guest_reg_signed* rs1s = (guest_reg_signed*)&cpu->regs[instruction->rs1];
	guest_reg_signed* rs2s = (guest_reg_signed*)&cpu->regs[instruction->rs2];
	guest_word_signed* rs1ws = (guest_word_signed*)((uint8_t*)&cpu->regs[instruction->rs1] + REG_WORD_OFFSET);
	guest_word_signed* rs2ws = (guest_word_signed*)((uint8_t*)&cpu->regs[instruction->rs2] + REG_WORD_OFFSET);

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)             \
	case ((OPCODE >> 2) | (F3 << 5) | (F7 << 8)): { \
		EXPR;                                   \
		break;                                  \
	}
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_I_IMM(MNEMONIC, OPCODE, F3, EXPR, F12S, F7S)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_U(MNEMONIC, OPCODE, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR)

		X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_I_IMM
#undef X_S
#undef X_B
#undef X_U
#undef X_J
		default:
			cpu_throw_exception(emu, EXC_ILL_INS, 0);
			break;
	}
}

static void cpu_execute_type_i(emulator_t* emu, const ins_t* instruction) {
	cpu_t* cpu = &emu->cpu;

	guest_reg* rd = &cpu->regs[instruction->rd];
	guest_reg* rs1 = &cpu->regs[instruction->rs1];
	guest_word* rs1w = (guest_word*)((uint8_t*)&cpu->regs[instruction->rs1] + REG_WORD_OFFSET);

	guest_reg_signed* rds = (guest_reg_signed*)&cpu->regs[instruction->rd];
	guest_reg_signed* rs1s = (guest_reg_signed*)&cpu->regs[instruction->rs1];
	guest_word_signed* rs1ws = (guest_word_signed*)((uint8_t*)&cpu->regs[instruction->rs1] + REG_WORD_OFFSET);

	int64_t imm = instruction->imm;

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)     \
	case ((OPCODE >> 2) | (F3 << 5)): { \
		EXPR;                       \
		break;                      \
	}
#define T(...) __VA_ARGS__
#define X_I_IMM(MNEMONIC, OPCODE, F3, EXPR, F12S, F7S)                                  \
	case ((OPCODE >> 2) | (F3 << 5)): {                                             \
		int64_t f12s[] = {F12S}, f7s[] = {F7S};                                 \
		bool found = false;                                                     \
		for (size_t i = 0; i < sizeof(f12s) / sizeof(f12s[0]) && !found; i++) { \
			if (f12s[i] == imm) {                                           \
				found = true;                                           \
			}                                                               \
		}                                                                       \
		for (size_t i = 0; i < sizeof(f7s) / sizeof(f7s[0]) && !found; i++) {   \
			if ((f7s[i] << 5) == (imm & 0xfe0)) {                           \
				found = true;                                           \
			}                                                               \
		}                                                                       \
		if (!found) {                                                           \
			cpu_throw_exception(emu, EXC_ILL_INS, 0);                       \
			break;                                                          \
		}                                                                       \
		EXPR;                                                                   \
		break;                                                                  \
	}
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_U(MNEMONIC, OPCODE, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR)

		X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef T
#undef X_I_IMM
#undef X_S
#undef X_B
#undef X_U
#undef X_J
		default:
			cpu_throw_exception(emu, EXC_ILL_INS, 0);
			break;
	}
}

static void cpu_execute_type_s(emulator_t* emu, const ins_t* instruction) {
	cpu_t* cpu = &emu->cpu;

	guest_reg* rs1 = &cpu->regs[instruction->rs1];
	guest_reg* rs2 = &cpu->regs[instruction->rs2];
	int64_t imm = instruction->imm;

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_I_IMM(MNEMONIC, OPCODE, F3, EXPR, F12S, F7S)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)     \
	case ((OPCODE >> 2) | (F3 << 5)): { \
		EXPR;                       \
		break;                      \
	}
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_U(MNEMONIC, OPCODE, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR)

		X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_I_IMM
#undef X_S
#undef X_B
#undef X_U
#undef X_J
		default:
			cpu_throw_exception(emu, EXC_ILL_INS, 0);
			break;
	}
}

static void cpu_execute_type_b(emulator_t* emu, const ins_t* instruction) {
	cpu_t* cpu = &emu->cpu;

	guest_reg* rs1 = &cpu->regs[instruction->rs1];
	guest_reg* rs2 = &cpu->regs[instruction->rs2];

	guest_reg_signed* rs1s = (guest_reg_signed*)&cpu->regs[instruction->rs1];
	guest_reg_signed* rs2s = (guest_reg_signed*)&cpu->regs[instruction->rs2];

	int64_t imm = instruction->imm;

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_I_IMM(MNEMONIC, OPCODE, F3, EXPR, F12S, F7S)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)     \
	case ((OPCODE >> 2) | (F3 << 5)): { \
		EXPR;                       \
		break;                      \
	}
#define X_U(MNEMONIC, OPCODE, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR)

		X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_I_IMM
#undef X_S
#undef X_B
#undef X_U
#undef X_J
		default:
			cpu_throw_exception(emu, EXC_ILL_INS, 0);
			break;
	}
}

static void cpu_execute_type_u(emulator_t* emu, const ins_t* instruction) {
	cpu_t* cpu = &emu->cpu;

	guest_reg* rd = &cpu->regs[instruction->rd];
	int64_t imm = instruction->imm;

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_I_IMM(MNEMONIC, OPCODE, F3, EXPR, F12S, F7S)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_U(MNEMONIC, OPCODE, EXPR) \
	case (OPCODE >> 2): {       \
		EXPR;               \
		break;              \
	}
#define X_J(MNEMONIC, OPCODE, EXPR)

		X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_I_IMM
#undef X_S
#undef X_B
#undef X_U
#undef X_J

		default:
			cpu_throw_exception(emu, EXC_ILL_INS, 0);
			break;
	}
}

static void cpu_execute_type_j(emulator_t* emu, const ins_t* instruction) {
	cpu_t* cpu = &emu->cpu;

	guest_reg* rd = &cpu->regs[instruction->rd];
	int64_t imm = instruction->imm;

#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_I_IMM(MNEMONIC, OPCODE, F3, EXPR, F12S, F7S)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_U(MNEMONIC, OPCODE, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR) EXPR;

	X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_I_IMM
#undef X_S
#undef X_B
#undef X_U
#undef X_J
}

#ifdef RISCV_EMULATOR_DYNAREC_X86_64_SUPPORT
static void cpu_execute_dynarec(emulator_t* emu) {
	assert(emu->cpu.dynarec_enabled);
	assert((emu->cpu.pc & 3) == 0);

	size_t cache_index = (emu->cpu.pc >> 2) & emu->cpu.instruction_cache_mask;
	dr_ins_t* cached_instruction = &emu->cpu.instruction_cache.as_dr_ins[cache_index];
	if (cached_instruction->tag != emu->cpu.pc || cached_instruction->native_code == NULL) {
		if (!dr_emit_block(emu, emu->cpu.pc)) {
			if (!emu->cpu.exception_pending) {
				cpu_throw_exception(emu, EXC_ILL_INS, 0);
			}
			return;
		}
	}
	assert(cached_instruction->tag == emu->cpu.pc);

	assert(emu->cpu.regs[0] == 0);

	emu->cpu.pc = dr_entry(emu, cached_instruction->native_code,
			       emu->cpu.regs, emu->cpu.pc);
}
#endif

void cpu_execute(emulator_t* emu) {
	/* The `exception_pending` flag is kept to true when an exception occured during the
	 * *current* instruction, we clean it on each new `cpu_execute`
	 */
	emu->cpu.exception_pending = false;
	emu->cpu.jump_pending = false;
	emu->cpu.tlb_or_cache_flush_pending = false;

	if ((emu->cpu.pc & 0x3) != 0) {
		cpu_throw_exception(emu, EXC_INS_ADDR_MISALIGNED, emu->cpu.pc);
		return;
	}

	if ((emu->device_update_iter & emu->device_update_iter_mask) == 0) {
		emu_update_mmio_devices(emu);
	}
	emu->device_update_iter++;
	cpu_check_interrupt(emu);

#ifdef RISCV_EMULATOR_DYNAREC_X86_64_SUPPORT
	if (emu->cpu.dynarec_enabled) {
		cpu_execute_dynarec(emu);
		return;
	}
#else
	assert(!emu->cpu.dynarec_enabled);
#endif

	ins_t* instruction;
	if (!cpu_decode_and_cache(emu, emu->cpu.pc, &instruction)) {
		if (!emu->cpu.exception_pending) {
			cpu_throw_exception(emu, EXC_ILL_INS, 0);
		}
		return;
	}

	// We check for x0: see the comment before clearing x0 at the end of the function
	assert(emu->cpu.regs[0] == 0);

	switch (instruction->type) {
		case INS_TYPE_R:
			cpu_execute_type_r(emu, instruction);
			break;
		case INS_TYPE_I:
			cpu_execute_type_i(emu, instruction);
			break;
		case INS_TYPE_S:
			cpu_execute_type_s(emu, instruction);
			break;
		case INS_TYPE_B:
			cpu_execute_type_b(emu, instruction);
			break;
		case INS_TYPE_U:
			cpu_execute_type_u(emu, instruction);
			break;
		case INS_TYPE_J:
			cpu_execute_type_j(emu, instruction);
			break;
		default:
			// TODO : better diag system
			fprintf(stderr, "Internal emulator error : invalid instruction type\n");
			abort();
			break;
	}

	if (!(emu->cpu.jump_pending || emu->cpu.exception_pending)) {
		emu->cpu.pc += 4;
	}

	/* zero (i.e. x0) is always set to 0 in RISC-V, some previous instructions might
	 * have tampered with this register for the sake of simplifying the emulation control
	 * flow
	 * (e.g. `j addr` == `jal x0, addr` might have set x0 to PC+4)
	 */
	emu->cpu.regs[0] = 0;
}

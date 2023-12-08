#include <assert.h>
#include <endian.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
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
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_U(MNEMONIC, OPCODE, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR)

		X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_S
#undef X_B
#undef X_U
#undef X_J
		default:
			fprintf(stderr, "Unsupported R instruction opcode_switch=%" PRIx16 "\n",
				instruction->opcode_switch);
			abort();
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
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_U(MNEMONIC, OPCODE, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR)

		X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_S
#undef X_B
#undef X_U
#undef X_J
		default:
			fprintf(stderr, "Unsupported I instruction opcode_switch=%" PRIx16 "\n",
				instruction->opcode_switch);
			abort();
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
#undef X_S
#undef X_B
#undef X_U
#undef X_J
		default:
			fprintf(stderr, "Unsupported S instruction opcode_switch=%" PRIx16 "\n",
				instruction->opcode_switch);
			abort();
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
#undef X_S
#undef X_B
#undef X_U
#undef X_J
		default:
			fprintf(stderr, "Unsupported B instruction opcode_switch=%" PRIx16 "\n",
				instruction->opcode_switch);
			abort();
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
#undef X_S
#undef X_B
#undef X_U
#undef X_J

		default:
			fprintf(stderr, "Unsupported U instruction opcode_switch=%" PRIx16 "\n",
				instruction->opcode_switch);
			abort();
			break;
	}
}

static void cpu_execute_type_j(emulator_t* emu, const ins_t* instruction) {
	cpu_t* cpu = &emu->cpu;

	guest_reg* rd = &cpu->regs[instruction->rd];
	int64_t imm = instruction->imm;

#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_U(MNEMONIC, OPCODE, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR) EXPR;

	X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_S
#undef X_B
#undef X_U
#undef X_J
}

void cpu_execute(emulator_t* emu) {
	assert(!emu->cpu.dynarec_enabled);

	if ((emu->cpu.pc & 0x3) != 0) {
		fprintf(stderr, "Unaligned PC=%016" PRIx64 "\n", emu->cpu.pc);
		abort();
	}

	ins_t* instruction;
	if (!cpu_decode_and_cache(emu, emu->cpu.pc, &instruction)) {
		fprintf(stderr, "Invalid instruction %08" PRIx32 " at PC=%016" PRIx64 "\n",
			emu_r32(emu, emu->cpu.pc), emu->cpu.pc);
		abort();
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

	if (emu->cpu.jump_pending) {
		emu->cpu.jump_pending = false;
	} else {
		emu->cpu.pc += 4;
	}

	/* zero (i.e. x0) is always set to 0 in RISC-V, some previous instructions might
	 * have tampered with this register for the sake of simplifying the emulation control
	 * flow
	 * (e.g. `j addr` == `jal x0, addr` might have set x0 to PC+4)
	 */
	emu->cpu.regs[0] = 0;
}

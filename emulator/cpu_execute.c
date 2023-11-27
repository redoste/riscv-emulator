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

	switch ((instruction->opcode >> 2) | (instruction->funct3 << 5) |
		(instruction->funct7 << 8)) {
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
			fprintf(stderr, "Unsupported R instruction opcode=%" PRIx8 " f3=%" PRIx8 " f7=%" PRIx8 "\n",
				instruction->opcode, instruction->funct3, instruction->funct7);
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

	switch ((instruction->opcode >> 2) | (instruction->funct3 << 5)) {
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
			fprintf(stderr, "Unsupported I instruction opcode=%" PRIx8 " f3=%" PRIx8 "\n",
				instruction->opcode, instruction->funct3);
			abort();
			break;
	}
}

static void cpu_execute_type_s(emulator_t* emu, const ins_t* instruction) {
	cpu_t* cpu = &emu->cpu;

	guest_reg* rs1 = &cpu->regs[instruction->rs1];
	guest_reg* rs2 = &cpu->regs[instruction->rs2];
	int64_t imm = instruction->imm;

	switch ((instruction->opcode >> 2) | (instruction->funct3 << 5)) {
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
			fprintf(stderr, "Unsupported S instruction opcode=%" PRIx8 " f3=%" PRIx8 "\n",
				instruction->opcode, instruction->funct3);
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

	switch ((instruction->opcode >> 2) | (instruction->funct3 << 5)) {
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
			fprintf(stderr, "Unsupported B instruction opcode=%" PRIx8 " f3=%" PRIx8 "\n",
				instruction->opcode, instruction->funct3);
			abort();
			break;
	}
}

static void cpu_execute_type_u(emulator_t* emu, const ins_t* instruction) {
	cpu_t* cpu = &emu->cpu;

	guest_reg* rd = &cpu->regs[instruction->rd];
	int64_t imm = instruction->imm;

	switch (instruction->opcode >> 2) {
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
			fprintf(stderr, "Unsupported U instruction opcode=%" PRIx8 "\n",
				instruction->opcode);
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
	if ((emu->cpu.pc & 0x3) != 0) {
		fprintf(stderr, "Unaligned PC=%016" PRIx64 "\n", emu->cpu.pc);
		abort();
	}

	uint32_t encoded_instruction = emu_r32(emu, emu->cpu.pc);
	ins_t instruction;
	if (!cpu_decode(encoded_instruction, &instruction)) {
		fprintf(stderr, "Invalid instruction %08" PRIx32 " at PC=%016" PRIx64 "\n",
			encoded_instruction, emu->cpu.pc);
		abort();
	}

	// We check for x0: see the comment before clearing x0 at the end of the function
	assert(emu->cpu.regs[0] == 0);

	switch (instruction.type) {
		case INS_TYPE_R:
			cpu_execute_type_r(emu, &instruction);
			break;
		case INS_TYPE_I:
			cpu_execute_type_i(emu, &instruction);
			break;
		case INS_TYPE_S:
			cpu_execute_type_s(emu, &instruction);
			break;
		case INS_TYPE_B:
			cpu_execute_type_b(emu, &instruction);
			break;
		case INS_TYPE_U:
			cpu_execute_type_u(emu, &instruction);
			break;
		case INS_TYPE_J:
			cpu_execute_type_j(emu, &instruction);
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

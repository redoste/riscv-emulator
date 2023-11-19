#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "emulator.h"
#include "isa.h"

static void cpu_execute_type_r(emulator_t* emu, const ins_t* instruction) {
	cpu_t* cpu = &emu->cpu;

	guest_reg* rd = &cpu->regs[instruction->rd];
	guest_reg* rs1 = &cpu->regs[instruction->rs1];
	guest_reg* rs2 = &cpu->regs[instruction->rs2];

	switch (instruction->opcode | (instruction->funct3 << 7) |
		(instruction->funct7 << 10)) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)         \
	case ((OPCODE) | (F3 << 7) | (F7 << 10)): { \
		EXPR;                               \
		break;                              \
	}
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR)

		X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_S
#undef X_B
#undef X_J
		default:
			fprintf(stderr, "Unsupported R instruction opcode=%hhx f3=%hhx f7=%hhx\n",
				instruction->opcode, instruction->funct3, instruction->funct7);
			abort();
			break;
	}
}

static void cpu_execute_type_i(emulator_t* emu, const ins_t* instruction) {
	cpu_t* cpu = &emu->cpu;

	guest_reg* rd = &cpu->regs[instruction->rd];
	guest_reg* rs1 = &cpu->regs[instruction->rs1];
	int64_t imm = instruction->imm;

	switch (instruction->opcode | (instruction->funct3 << 7)) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR) \
	case ((OPCODE) | (F3 << 7)): {  \
		EXPR;                   \
		break;                  \
	}
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR)

		X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_S
#undef X_B
#undef X_J
		default:
			fprintf(stderr, "Unsupported I instruction opcode=%hhx f3=%hhx\n",
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

	switch (instruction->opcode | (instruction->funct3 << 7)) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_S(MNEMONIC, OPCODE, F3, EXPR) \
	case ((OPCODE) | (F3 << 7)): {  \
		EXPR;                   \
		break;                  \
	}
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR)

		X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_S
#undef X_B
#undef X_J
		default:
			fprintf(stderr, "Unsupported S instruction opcode=%hhx f3=%hhx\n",
				instruction->opcode, instruction->funct3);
			abort();
			break;
	}
}

static void cpu_execute_type_b(emulator_t* emu, const ins_t* instruction) {
	cpu_t* cpu = &emu->cpu;

	guest_reg* rs1 = &cpu->regs[instruction->rs1];
	guest_reg* rs2 = &cpu->regs[instruction->rs2];
	int64_t imm = instruction->imm;

	switch (instruction->opcode | (instruction->funct3 << 7)) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR) \
	case ((OPCODE) | (F3 << 7)): {  \
		EXPR;                   \
		break;                  \
	}
#define X_J(MNEMONIC, OPCODE, EXPR)

		X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_S
#undef X_B
#undef X_J
		default:
			fprintf(stderr, "Unsupported B instruction opcode=%hhx f3=%hhx\n",
				instruction->opcode, instruction->funct3);
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
#define X_J(MNEMONIC, OPCODE, EXPR) EXPR;

	X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_S
#undef X_B
#undef X_J
}

void cpu_execute(emulator_t* emu) {
	if ((emu->cpu.pc & 0x3) != 0) {
		fprintf(stderr, "Unaligned PC=%016lx\n", emu->cpu.pc);
		abort();
	}

	uint32_t encoded_instruction = emu_r32(emu, emu->cpu.pc);
	ins_t instruction;
	if (!cpu_decode(encoded_instruction, &instruction)) {
		fprintf(stderr, "Invalid instruction %08x at PC=%016lx\n",
			encoded_instruction, emu->cpu.pc);
		abort();
	}

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
}
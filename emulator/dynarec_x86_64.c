#ifdef RISCV_EMULATOR_DYNAREC_X86_64_SUPPORT

#include <assert.h>
#include <stdbool.h>
#include <sys/mman.h>

#include "dynarec_x86_64.h"
#include "emulator.h"
#include "isa.h"

static inline bool dr_emit_x86_code(const dr_x86_code_t* x86_code, const ins_t* instruction, dr_block_t* block) {
	if (block->pos + x86_code->code_size >= (DYNAREC_PAGE_SIZE - DYNAREC_PROLOGUE_SIZE)) {
		return false;
	}

	memcpy(block->page + block->pos, x86_code->code, x86_code->code_size);
	if (x86_code->rs1_reloc != -1) {
		block->page[block->pos + x86_code->rs1_reloc] = (instruction->rs1 - 16) * 8;
	}
	if (x86_code->rs2_reloc != -1) {
		block->page[block->pos + x86_code->rs2_reloc] = (instruction->rs2 - 16) * 8;
	}
	if (x86_code->rd_reloc != -1) {
		block->page[block->pos + x86_code->rd_reloc] = (instruction->rd - 16) * 8;
	}
	if (x86_code->imm_reloc != -1) {
		*(int32_t*)(&block->page[block->pos + x86_code->imm_reloc]) = instruction->imm;
	}

	// TODO : track block->pc @ block->pos in the instruction cache
	block->pos += x86_code->code_size;

	return true;
}

static inline bool dr_emit_type_r(const ins_t* instruction, dr_block_t* block) {
	size_t zero_selector = ((instruction->rs1 == 0) << 0) |
			       ((instruction->rs2 == 0) << 1) |
			       ((instruction->rd == 0) << 2);

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)                                \
	case ((OPCODE >> 2) | (F3 << 5) | (F7 << 8)): {                    \
		assert(DR_X86_##MNEMONIC[0].code_size > 0);                \
		return dr_emit_x86_code(&DR_X86_##MNEMONIC[zero_selector], \
					instruction, block);               \
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

static inline bool dr_emit_type_i(const ins_t* instruction, dr_block_t* block) {
	size_t zero_selector = ((instruction->rs1 == 0) << 0) |
			       ((instruction->rd == 0) << 1);

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)                                    \
	case ((OPCODE >> 2) | (F3 << 5)): {                                \
		assert(DR_X86_##MNEMONIC[0].code_size > 0);                \
		return dr_emit_x86_code(&DR_X86_##MNEMONIC[zero_selector], \
					instruction, block);               \
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

static inline bool dr_emit_type_s(const ins_t* instruction, dr_block_t* block) {
	size_t zero_selector = ((instruction->rs1 == 0) << 0) |
			       ((instruction->rs2 == 0) << 1);

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)                                    \
	case ((OPCODE >> 2) | (F3 << 5)): {                                \
		assert(DR_X86_##MNEMONIC[0].code_size > 0);                \
		return dr_emit_x86_code(&DR_X86_##MNEMONIC[zero_selector], \
					instruction, block);               \
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

static inline bool dr_emit_type_b(const ins_t* instruction, dr_block_t* block) {
	size_t zero_selector = ((instruction->rs1 == 0) << 0) |
			       ((instruction->rs2 == 0) << 1);

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)                                    \
	case ((OPCODE >> 2) | (F3 << 5)): {                                \
		assert(DR_X86_##MNEMONIC[0].code_size > 0);                \
		return dr_emit_x86_code(&DR_X86_##MNEMONIC[zero_selector], \
					instruction, block);               \
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

static inline bool dr_emit_type_u(const ins_t* instruction, dr_block_t* block) {
	size_t zero_selector = (instruction->rd == 0);

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_U(MNEMONIC, OPCODE, EXPR)                                        \
	case (OPCODE >> 2): {                                              \
		assert(DR_X86_##MNEMONIC[0].code_size > 0);                \
		return dr_emit_x86_code(&DR_X86_##MNEMONIC[zero_selector], \
					instruction, block);               \
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

static inline bool dr_emit_type_j(const ins_t* instruction, dr_block_t* block) {
	size_t zero_selector = (instruction->rd == 0);

#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_U(MNEMONIC, OPCODE, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR)                                \
	assert(DR_X86_##MNEMONIC[0].code_size > 0);                \
	return dr_emit_x86_code(&DR_X86_##MNEMONIC[zero_selector], \
				instruction, block);

	X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_S
#undef X_B
#undef X_U
#undef X_J
}

bool dr_emit_block(emulator_t* emu, guest_paddr base) {
	assert((base & 3) == 0);

	uint8_t* page = mmap(NULL, DYNAREC_PAGE_SIZE, PROT_READ | PROT_WRITE,
			     MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
	if (page == MAP_FAILED) {
		return false;
	}

	dr_block_t block = {
		.page = page,
		.pos = 0,
		.pc = base,
	};

	bool cont = true;
	while (cont) {
		/* TODO : properly report to the caller if we failed because the instruction
		 *        is invalid or because we can't read memory
		 */
		uint32_t encoded_instruction = emu_r32(emu, block.pc);
		ins_t instruction;
		if (!cpu_decode(encoded_instruction, &instruction)) {
			break;
		}

		switch (instruction.type) {
			case INS_TYPE_R:
				cont &= dr_emit_type_r(&instruction, &block);
				break;
			case INS_TYPE_I:
				cont &= dr_emit_type_i(&instruction, &block);
				break;
			case INS_TYPE_S:
				cont &= dr_emit_type_s(&instruction, &block);
				break;
			case INS_TYPE_B:
				cont &= dr_emit_type_b(&instruction, &block);
				break;
			case INS_TYPE_U:
				cont &= dr_emit_type_u(&instruction, &block);
				break;
			case INS_TYPE_J:
				cont &= dr_emit_type_j(&instruction, &block);
				break;
			default:
				fprintf(stderr, "Internal emulator error : invalid instruction type\n");
				abort();
				break;
		}

		if (instruction.opcode_switch == (OPCODE_JAL >> 2) ||
		    instruction.opcode_switch == ((OPCODE_JALR >> 2) | (F3_JALR << 5))) {
			break;
		}
		block.pc += 4;
	}

	assert(block.pos + DYNAREC_PROLOGUE_SIZE <= DYNAREC_PAGE_SIZE);
	// jmp r11
	block.page[block.pos + 0] = 0x41;
	block.page[block.pos + 1] = 0xff;
	block.page[block.pos + 2] = 0xe3;

	if (mprotect(page, DYNAREC_PAGE_SIZE, PROT_READ | PROT_EXEC) < 0) {
		perror("mprotect");
		fprintf(stderr, "Internal emulator error : unable to mark page as PROT_EXEC\n");
		abort();
	}

	return true;
}

#endif

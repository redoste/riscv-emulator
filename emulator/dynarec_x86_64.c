#ifdef RISCV_EMULATOR_DYNAREC_X86_64_SUPPORT

#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "dynarec_x86_64.h"
#include "emulator.h"
#include "isa.h"

static inline bool dr_emit_x86_code(emulator_t* emu, const dr_x86_code_t* x86_code, const ins_t* instruction, dr_block_t* block) {
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
	if (x86_code->rs1uimm_reloc != -1) {
		block->page[block->pos + x86_code->rs1uimm_reloc] = instruction->rs1;
	}

	size_t cache_index = (block->pc >> 2) & emu->cpu.instruction_cache_mask;
	dr_ins_t* cached_instruction = &emu->cpu.instruction_cache.as_dr_ins[cache_index];

	if (cached_instruction->native_code != NULL) {
		if (cached_instruction->block_entry == block->base) {
			/* The current block is too big to fit in the instruction cache and we're
			 * looping back to an other instruction alread owned by this block
			 */
			return false;
		}
		cpu_invalidate_instruction_cache(emu, cached_instruction->tag);
		assert(cached_instruction->native_code == NULL);
	}

	cached_instruction->tag = block->pc;
	cached_instruction->block_entry = block->base;
	cached_instruction->native_code = block->page + block->pos;

	block->pos += x86_code->code_size;

	return true;
}

static inline bool dr_emit_type_r(emulator_t* emu, const ins_t* instruction, dr_block_t* block) {
	size_t zero_selector = ((instruction->rs1 == 0) << 0) |
			       ((instruction->rs2 == 0) << 1) |
			       ((instruction->rd == 0) << 2);

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)                                \
	case ((OPCODE >> 2) | (F3 << 5) | (F7 << 8)): {                    \
		assert(DR_X86_##MNEMONIC[0].code_size > 0);                \
		return dr_emit_x86_code(emu,                               \
					&DR_X86_##MNEMONIC[zero_selector], \
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

static inline bool dr_emit_type_i(emulator_t* emu, const ins_t* instruction, dr_block_t* block) {
	size_t zero_selector = ((instruction->rs1 == 0) << 0) |
			       ((instruction->rd == 0) << 1);

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)                                    \
	case ((OPCODE >> 2) | (F3 << 5)): {                                \
		assert(DR_X86_##MNEMONIC[0].code_size > 0);                \
		return dr_emit_x86_code(emu,                               \
					&DR_X86_##MNEMONIC[zero_selector], \
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

static inline bool dr_emit_type_s(emulator_t* emu, const ins_t* instruction, dr_block_t* block) {
	size_t zero_selector = ((instruction->rs1 == 0) << 0) |
			       ((instruction->rs2 == 0) << 1);

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)                                    \
	case ((OPCODE >> 2) | (F3 << 5)): {                                \
		assert(DR_X86_##MNEMONIC[0].code_size > 0);                \
		return dr_emit_x86_code(emu,                               \
					&DR_X86_##MNEMONIC[zero_selector], \
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

static inline bool dr_emit_type_b(emulator_t* emu, const ins_t* instruction, dr_block_t* block) {
	size_t zero_selector = ((instruction->rs1 == 0) << 0) |
			       ((instruction->rs2 == 0) << 1);

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)                                    \
	case ((OPCODE >> 2) | (F3 << 5)): {                                \
		assert(DR_X86_##MNEMONIC[0].code_size > 0);                \
		return dr_emit_x86_code(emu,                               \
					&DR_X86_##MNEMONIC[zero_selector], \
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

static inline bool dr_emit_type_u(emulator_t* emu, const ins_t* instruction, dr_block_t* block) {
	size_t zero_selector = (instruction->rd == 0);

	switch (instruction->opcode_switch) {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_U(MNEMONIC, OPCODE, EXPR)                                        \
	case (OPCODE >> 2): {                                              \
		assert(DR_X86_##MNEMONIC[0].code_size > 0);                \
		return dr_emit_x86_code(emu,                               \
					&DR_X86_##MNEMONIC[zero_selector], \
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

static inline bool dr_emit_type_j(emulator_t* emu, const ins_t* instruction, dr_block_t* block) {
	size_t zero_selector = (instruction->rd == 0);

#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR)
#define X_I(MNEMONIC, OPCODE, F3, EXPR)
#define X_S(MNEMONIC, OPCODE, F3, EXPR)
#define X_B(MNEMONIC, OPCODE, F3, EXPR)
#define X_U(MNEMONIC, OPCODE, EXPR)
#define X_J(MNEMONIC, OPCODE, EXPR)                                \
	assert(DR_X86_##MNEMONIC[0].code_size > 0);                \
	return dr_emit_x86_code(emu,                               \
				&DR_X86_##MNEMONIC[zero_selector], \
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
	assert(emu->cpu.dynarec_enabled);
	assert((base & 3) == 0);

	uint8_t* page = mmap(NULL, DYNAREC_PAGE_SIZE, PROT_READ | PROT_WRITE,
			     MAP_ANONYMOUS | MAP_PRIVATE, 0, 0);
	if (page == MAP_FAILED) {
		return false;
	}

	dr_block_t block = {
		.page = page,
		.pos = 0,
		.base = base,
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
				cont &= dr_emit_type_r(emu, &instruction, &block);
				break;
			case INS_TYPE_I:
				cont &= dr_emit_type_i(emu, &instruction, &block);
				break;
			case INS_TYPE_S:
				cont &= dr_emit_type_s(emu, &instruction, &block);
				break;
			case INS_TYPE_B:
				cont &= dr_emit_type_b(emu, &instruction, &block);
				break;
			case INS_TYPE_U:
				cont &= dr_emit_type_u(emu, &instruction, &block);
				break;
			case INS_TYPE_J:
				cont &= dr_emit_type_j(emu, &instruction, &block);
				break;
			default:
				fprintf(stderr, "Internal emulator error : invalid instruction type\n");
				abort();
				break;
		}

		if (instruction.type == INS_TYPE_J ||
		    instruction.opcode_switch == ((OPCODE_JALR >> 2) | (F3_JALR << 5))) {
			break;
		}
		block.pc += 4;
	}

	if (block.pos == 0) {
		munmap(block.page, DYNAREC_PAGE_SIZE);
		return false;
	}

	assert(block.pos + DYNAREC_PROLOGUE_SIZE <= DYNAREC_PAGE_SIZE);
	// jmp r10
	block.page[block.pos + 0] = 0x41;
	block.page[block.pos + 1] = 0xff;
	block.page[block.pos + 2] = 0xe2;

	if (mprotect(page, DYNAREC_PAGE_SIZE, PROT_READ | PROT_EXEC) < 0) {
		perror("mprotect");
		fprintf(stderr, "Internal emulator error : unable to mark page as PROT_EXEC\n");
		abort();
	}

	return true;
}

void dr_free(emulator_t* emu) {
	assert(emu->cpu.dynarec_enabled);

	size_t instruction_cache_size = emu->cpu.instruction_cache_mask + 1;
	for (size_t i = 0; i < instruction_cache_size; i++) {
		dr_ins_t* cached_instruction = &emu->cpu.instruction_cache.as_dr_ins[i];
		if (cached_instruction->native_code != NULL) {
			uintptr_t page_base = (uintptr_t)cached_instruction->native_code & DYNAREC_PAGE_MASK;
			munmap((void*)page_base, DYNAREC_PAGE_SIZE);
			cached_instruction->native_code = NULL;
		}
	}
}

#endif

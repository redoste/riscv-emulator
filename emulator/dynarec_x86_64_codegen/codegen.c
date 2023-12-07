#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "assembler.h"
#include "codegen.h"
#include "x86_isa.h"

static size_t codegen_current_ins;

void codegen_start_ins(const char* mnemonic) {
	printf("const dr_x86_code_t DR_X86_%s[] = {\n", mnemonic);
	codegen_current_ins = 0;
}

void codegen_end_ins(void) {
	if (codegen_current_ins == 0) {
		printf("\t{0},\n");
	}
	printf("};\n");
}

#define CODEGEN_BUFFER_CAPACITY 64
static struct {
	size_t pos;
	ssize_t rs1_reloc;
	ssize_t rs2_reloc;
	ssize_t rd_reloc;
	size_t imm_reloc;
	uint8_t buffer[CODEGEN_BUFFER_CAPACITY];
} codegen_current_line;

void codegen_start_line(bool b0, bool b1, bool b2) {
	uint8_t selector = b0 | (b1 << 1) | (b2 << 2);
	printf("\t[%" PRId8 "] = {(uint8_t*)\"", selector);

	codegen_current_line.pos = 0;
	codegen_current_line.rs1_reloc = -1;
	codegen_current_line.rs2_reloc = -1;
	codegen_current_line.rd_reloc = -1;
	codegen_current_line.imm_reloc = -1;
}

void codegen_asm(x86_mnemonic_t mnemonic, x86_operand_t dst, x86_operand_t src, codegen_reloc_type_t reloc_type) {
	size_t reloc_pos;
	size_t written = asm_x86_ins(codegen_current_line.buffer + codegen_current_line.pos,
				     CODEGEN_BUFFER_CAPACITY - codegen_current_line.pos,
				     &reloc_pos, mnemonic, dst, src);

	if (written <= 0) {
		fprintf(stderr, "Unable to assemble %d (%" PRIx64 ", %" PRIx64 ")\n", mnemonic, dst, src);
		abort();
	}

	switch (reloc_type) {
		case CODEGEN_RELOC_RS1:
			codegen_current_line.rs1_reloc = codegen_current_line.pos + reloc_pos;
			break;
		case CODEGEN_RELOC_RS2:
			codegen_current_line.rs2_reloc = codegen_current_line.pos + reloc_pos;
			break;
		case CODEGEN_RELOC_RD:
			codegen_current_line.rd_reloc = codegen_current_line.pos + reloc_pos;
			break;
		case CODEGEN_RELOC_IMM:
			codegen_current_line.imm_reloc = codegen_current_line.pos + reloc_pos;
			break;
		default:
			break;
	}

	codegen_current_line.pos += written;
	assert(codegen_current_line.pos < CODEGEN_BUFFER_CAPACITY);
}

void codegen_end_line(void) {
	for (size_t i = 0; i < codegen_current_line.pos; i++) {
		printf("\\x%02" PRIx8, codegen_current_line.buffer[i]);
	}
	printf("\", %zu, %zd, %zd, %zd, %zd},\n", codegen_current_line.pos,
	       codegen_current_line.rs1_reloc, codegen_current_line.rs2_reloc,
	       codegen_current_line.rd_reloc, codegen_current_line.imm_reloc);

	codegen_current_ins++;
}

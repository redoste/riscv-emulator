#include <stdio.h>
#include <string.h>

#include "assembler.h"
#include "lexer.h"

int main(int argc, char** argv) {
	/* ./riscv-assembler <ASSEMBLER INPUT> <HEX OUTPUT> */
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <ASSEMBLER INPUT> <HEX OUTPUT>\n", argv[0]);
		fprintf(stderr, "       (use \"-\" for stdin or stdout)\n");
		return 1;
	}

	char* asm_input_filename = argv[1];
	char* hex_output_filename = argv[2];

	FILE* input_file;
	if (strcmp(asm_input_filename, "-") == 0) {
		input_file = stdin;
	} else {
		input_file = fopen(asm_input_filename, "r");
	}
	if (input_file == NULL) {
		perror("fopen");
		return 1;
	}

	FILE* output_file;
	if (strcmp(hex_output_filename, "-") == 0) {
		output_file = stdout;
	} else {
		output_file = fopen(hex_output_filename, "w");
	}
	if (output_file == NULL) {
		perror("fopen");
		fclose(input_file);
		return 1;
	}

	lexer_t lexer;
	lexer_create(&lexer, input_file, asm_input_filename);

	token_t token;
	int res = 0;
	do {
		if (!lexer_next_expected(&lexer, &token, TT_INS_MNEMONIC, true)) {
			res = 1;
			break;
		}

		if (token.type == TT_INS_MNEMONIC) {
			uint32_t instruction;
			if (!assembler_assemble_instruction(token.as_ins_mnemonic, &lexer, &instruction)) {
				res = 1;
				break;
			}
			fprintf(output_file, "%08x\n", instruction);
		}
	} while (token.type != TT_EOF);

	fclose(input_file);
	fclose(output_file);
	return res;
}

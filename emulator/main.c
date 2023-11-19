#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "emulator.h"
#include "isa.h"

#define ROM_BASE 0x80000000
#define ROM_SIZE 0x2000
#define RAM_BASE 0xc0000000
#define RAM_SIZE 0x2000

int main(int argc, char** argv) {
	/* ./riscv-emulator <HEX INPUT> <EMULATION OUTPUT> */
	if (argc != 3) {
		fprintf(stderr, "Usage: %s <HEX INPUT> <EMULATION OUTPUT>\n", argv[0]);
		fprintf(stderr, "       (use \"-\" for stdin or stdout)\n");
		return 1;
	}

	char* hex_input_filename = argv[1];
	char* emu_output_filename = argv[2];

	FILE* input_file;
	if (strcmp(hex_input_filename, "-") == 0) {
		input_file = stdin;
	} else {
		input_file = fopen(hex_input_filename, "r");
	}
	if (input_file == NULL) {
		perror("fopen");
		return 1;
	}

	emulator_t emu;
	emu_create(&emu, ROM_BASE, ROM_SIZE, RAM_BASE, RAM_SIZE);

	guest_paddr max_rom_code_addr = ROM_BASE;
	do {
		char buffer[32] = {0};
		if (fgets(buffer, sizeof(buffer) - 1, input_file)) {
			uint32_t instruction = strtoull(buffer, NULL, 16);
			emu_w32(&emu, max_rom_code_addr, instruction);
			max_rom_code_addr += 4;
		} else if (!feof(input_file)) {
			perror("fgets");
			fclose(input_file);
			emu_destroy(&emu);
			return 1;
		}
	} while (!feof(input_file));
	fclose(input_file);

	while (emu.cpu.pc >= ROM_BASE && emu.cpu.pc < max_rom_code_addr) {
		cpu_execute(&emu);
	}

	FILE* output_file;
	if (strcmp(emu_output_filename, "-") == 0) {
		output_file = stdout;
	} else {
		output_file = fopen(emu_output_filename, "w");
	}
	if (output_file == NULL) {
		perror("fopen");
		return 1;
	}

	for (size_t i = 0; i < REG_COUNT; i++) {
		fprintf(output_file, "x%zu: 0x%lx\n", i, emu.cpu.regs[i]);
	}
	fclose(output_file);

	emu_destroy(&emu);
	return 0;
}

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "emulator.h"
#include "isa.h"

#define DEFAULT_ROM_BASE 0x80000000
#define DEFAULT_ROM_SIZE 0x2000
#define DEFAULT_RAM_BASE 0xc0000000
#define DEFAULT_RAM_SIZE 0x2000

static int usage(const char* argv0) {
	fprintf(stderr,
		"Simple mode:\n"
		"Usage:   %s <HEX INPUT> <EMULATION OUTPUT>\n"
		"         (use \"-\" for stdin or stdout)\n"
		"\n"
		"Advanced mode:\n"
		"Usage:   %s --advanced <ROM FILE> [options]\n"
		"Options:\n"
		"    --rom-base 0x[ROM BASE] : Base address of the ROM (default 0x%08x)\n"
		"    --rom-size 0x[ROM SIZE] : Size of the ROM (defaut 0x%08x)\n"
		"    --ram-base 0x[RAM BASE] : Base address of the RAM (default 0x%08x)\n"
		"    --ram-size 0x[RAM SIZE] : Size of the ROM (default 0x%08x)\n",
		argv0, argv0, DEFAULT_ROM_BASE, DEFAULT_ROM_SIZE, DEFAULT_RAM_BASE, DEFAULT_RAM_SIZE);
	return 1;
}

static int main_simple(const char* hex_input_filename, const char* emu_output_filename) {
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
	emu_create(&emu, DEFAULT_ROM_BASE, DEFAULT_ROM_SIZE, DEFAULT_RAM_BASE, DEFAULT_RAM_SIZE);

	guest_paddr max_rom_code_addr = DEFAULT_ROM_BASE;
	do {
		char buffer[32] = {0};
		if (fgets(buffer, sizeof(buffer) - 1, input_file)) {
			uint32_t instruction = strtoull(buffer, NULL, 16);
			emu_w32(&emu, max_rom_code_addr, instruction);
			max_rom_code_addr += sizeof(uint32_t);
		} else if (!feof(input_file)) {
			perror("fgets");
			fclose(input_file);
			emu_destroy(&emu);
			return 1;
		}
	} while (!feof(input_file) && max_rom_code_addr < (DEFAULT_ROM_BASE + DEFAULT_ROM_SIZE));
	fclose(input_file);

	while (emu.cpu.pc >= DEFAULT_ROM_BASE && emu.cpu.pc < max_rom_code_addr) {
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

static bool parse_hex_argv(const char* argv, unsigned long* res) {
	if (strncmp(argv, "0x", 2) != 0) {
		return false;
	}
	char* endptr;
	*res = strtoul(&argv[2], &endptr, 16);
	return endptr == argv + strlen(argv);
}

static int main_advanced(int argc, char** argv) {
	assert(argc >= 3);
	ssize_t argc_iter = 1;

	const char* rom_file = NULL;
	guest_paddr rom_base = DEFAULT_ROM_BASE, ram_base = DEFAULT_RAM_BASE;
	size_t rom_size = DEFAULT_ROM_SIZE, ram_size = DEFAULT_RAM_SIZE;

	while (argc_iter < argc) {
		if (strcmp(argv[argc_iter], "--advanced") == 0) {
			argc_iter++;
			rom_file = argv[argc_iter++];
		}
#define PARSE_HEX_ARG(ARG_NAME, VALUE)                             \
	else if (strcmp(argv[argc_iter], (ARG_NAME)) == 0) {       \
		argc_iter++;                                       \
		if (argc_iter >= argc ||                           \
		    !parse_hex_argv(argv[argc_iter++], (VALUE))) { \
			return usage(argv[0]);                     \
		}                                                  \
	}
		PARSE_HEX_ARG("--rom-base", &rom_base)
		PARSE_HEX_ARG("--rom-size", &rom_size)
		PARSE_HEX_ARG("--ram-base", &ram_base)
		PARSE_HEX_ARG("--ram-size", &ram_size)
#undef PARSE_HEX_ARG
		else {
			return usage(argv[0]);
		}
	}
	assert(rom_file != NULL);

	FILE* input_file = fopen(rom_file, "r");
	if (input_file == NULL) {
		perror("fopen");
		return 1;
	}
	if (fseek(input_file, 0, SEEK_END) != 0) {
		perror("fseek");
		fclose(input_file);
		return 1;
	}
	size_t file_size = ftell(input_file);
	fseek(input_file, 0, SEEK_SET);

	if (file_size > rom_size) {
		fprintf(stderr, "The ROM file is too big to fit in the specified ROM size\n");
		fclose(input_file);
		return 1;
	}

	emulator_t emu;
	emu_create(&emu, rom_base, rom_size, ram_base, ram_size);

	if (fread(emu.rom.pool, 1, file_size, input_file) != file_size) {
		perror("fread");
		fclose(input_file);
		emu_destroy(&emu);
		return 1;
	}
	fclose(input_file);

	// TODO : add proper exit
	while (emu.cpu.pc >= rom_base && emu.cpu.pc < rom_base + rom_size) {
		cpu_execute(&emu);
	}

	emu_destroy(&emu);
	return 0;
}

int main(int argc, char** argv) {
	if (argc < 3) {
		return usage(argv[0]);
	}

	if (strcmp(argv[1], "--advanced") == 0) {
		return main_advanced(argc, argv);
	} else {
		return main_simple(argv[1], argv[2]);
	}
}

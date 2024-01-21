#include <assert.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "devices.h"
#include "emulator.h"
#include "isa.h"

#define DEFAULT_ROM_BASE             0x80000000
#define DEFAULT_ROM_SIZE             0x2000
#define DEFAULT_RAM_BASE             0xc0000000
#define DEFAULT_RAM_SIZE             0x2000
#define DEFAULT_CACHE_BITS           16
#define DEFAULT_DEVICE_UPDATE_PERIOD 18

static int usage(const char* argv0) {
	fprintf(stderr,
		"Simple mode:\n"
		"Usage:   %s <HEX INPUT> <EMULATION OUTPUT>\n"
		"         (use \"-\" for stdin or stdout)\n"
		"\n"
		"Advanced mode:\n"
		"Usage:   %s --advanced <ROM FILE> [options]\n"
		"Options:\n"
		"    --rom-base 0x[ROM BASE]  : Base address of the ROM (default 0x%08x)\n"
		"    --rom-size 0x[ROM SIZE]  : Size of the ROM (defaut 0x%08x)\n"
		"    --ram-base 0x[RAM BASE]  : Base address of the RAM (default 0x%08x)\n"
		"    --ram-size 0x[RAM SIZE]  : Size of the ROM (default 0x%08x)\n"
		"    --user-only              : Keep the emulated CPU in U-mode and expose emulator calls through ecall\n"
		"                               (as used by the provided `DOOM` port)\n"
		"    --virt [HDD IMAGE]       : Create a QEMU \"virt\" style machine following the device tree described\n"
		"                               in `linux/emulator.dts`\n"
#ifdef RISCV_EMULATOR_DYNAREC_X86_64_SUPPORT
		"    --dynarec                : Enable dynamic recompilation to x86-64 assembly\n"
#endif
		"    --cache-bits [BITS]      : Number of significant bits for the different caches (default %d)\n"
		"    --dev-update-period [T]  : Device update period in powers of 2 (default %d)\n",
		argv0, argv0,
		DEFAULT_ROM_BASE, DEFAULT_ROM_SIZE,
		DEFAULT_RAM_BASE, DEFAULT_RAM_SIZE,
		DEFAULT_CACHE_BITS, DEFAULT_DEVICE_UPDATE_PERIOD);
	return 1;
}

#define SIMPLE_ROM_BASE 0x0
#define SIMPLE_ROM_SIZE (16 << 10)

#ifdef RISCV_EMULATOR_DYNAREC_X86_64_SUPPORT
#define SIMPLE_DYNAREC_ENABLED true
#else
#define SIMPLE_DYNAREC_ENABLED false
#endif

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
	emu_create(&emu, SIMPLE_ROM_BASE,
		   DEFAULT_CACHE_BITS, DEFAULT_DEVICE_UPDATE_PERIOD,
		   SIMPLE_DYNAREC_ENABLED, true);
	bool map_ret = emu_map_memory(&emu, SIMPLE_ROM_BASE, SIMPLE_ROM_SIZE);
	map_ret &= emu_map_memory(&emu, DEFAULT_RAM_BASE, DEFAULT_RAM_SIZE);
	assert(map_ret);

	guest_paddr max_rom_code_addr = SIMPLE_ROM_BASE;
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
	} while (!feof(input_file) && max_rom_code_addr < (SIMPLE_ROM_BASE + SIMPLE_ROM_SIZE));
	fclose(input_file);

	emu.cpu.regs[2] = SIMPLE_ROM_SIZE;  // sp
	while (emu.cpu.pc >= SIMPLE_ROM_BASE && emu.cpu.pc < max_rom_code_addr) {
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
		fprintf(output_file, "x%zu: 0x%" PRIx64 "\n", i, emu.cpu.regs[i]);
	}
	fclose(output_file);

	emu_destroy(&emu);
	return 0;
}

static bool parse_num_argv(const char* argv, uint64_t* res) {
	if (strncmp(argv, "0x", 2) == 0) {
		char* endptr;
		*res = strtoull(&argv[2], &endptr, 16);
		return endptr == argv + strlen(argv);
	}

	char* endptr;
	*res = strtoull(argv, &endptr, 10);
	return endptr == argv + strlen(argv);
}

static int main_advanced(int argc, char** argv) {
	assert(argc >= 3);
	ssize_t argc_iter = 1;

	const char *rom_file = NULL, *hdd_file = NULL;
	guest_paddr rom_base = DEFAULT_ROM_BASE, ram_base = DEFAULT_RAM_BASE;
	size_t rom_size = DEFAULT_ROM_SIZE, ram_size = DEFAULT_RAM_SIZE;
	size_t cache_bits = DEFAULT_CACHE_BITS, device_update_period = DEFAULT_DEVICE_UPDATE_PERIOD;
	bool dynarec_enabled = false, user_only_mode = false;

	while (argc_iter < argc) {
		if (strcmp(argv[argc_iter], "--advanced") == 0) {
			argc_iter++;
			rom_file = argv[argc_iter++];
		}
#define PARSE_NUM_ARG(ARG_NAME, VALUE)                                \
	else if (strcmp(argv[argc_iter], (ARG_NAME)) == 0) {          \
		argc_iter++;                                          \
		uint64_t value_u64;                                   \
		if (argc_iter >= argc ||                              \
		    !parse_num_argv(argv[argc_iter++], &value_u64)) { \
			return usage(argv[0]);                        \
		}                                                     \
		*(VALUE) = value_u64;                                 \
	}
		PARSE_NUM_ARG("--rom-base", &rom_base)
		PARSE_NUM_ARG("--rom-size", &rom_size)
		PARSE_NUM_ARG("--ram-base", &ram_base)
		PARSE_NUM_ARG("--ram-size", &ram_size)
		PARSE_NUM_ARG("--cache-bits", &cache_bits)
		PARSE_NUM_ARG("--dev-update-period", &device_update_period)
#undef PARSE_NUM_ARG
#ifdef RISCV_EMULATOR_DYNAREC_X86_64_SUPPORT
		else if (strcmp(argv[argc_iter], "--dynarec") == 0) {
			argc_iter++;
			dynarec_enabled = true;
		}
#endif
		else if (strcmp(argv[argc_iter], "--user-only") == 0) {
			argc_iter++;
			user_only_mode = true;
		}
		else if (strcmp(argv[argc_iter], "--virt") == 0) {
			argc_iter++;
			hdd_file = argv[argc_iter++];
		}
		else {
			return usage(argv[0]);
		}
	}
	assert(rom_file != NULL);

	FILE* input_file = fopen(rom_file, "rb");
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
	emu_create(&emu, rom_base, cache_bits, device_update_period, dynarec_enabled, user_only_mode);
	if (!emu_map_memory(&emu, rom_base, rom_size) ||
	    !emu_map_memory(&emu, ram_base, ram_size)) {
		fprintf(stderr,
			"Unable to map the emulated memory\n"
			"Make sure it's properly aligned to page boundaries and is using cannonical addresses\n");
		fclose(input_file);
		emu_destroy(&emu);
		return 1;
	}

	uint8_t* rom_content = malloc(file_size);
	assert(rom_content != NULL);
	if (fread(rom_content, 1, file_size, input_file) != file_size) {
		perror("fread");
		fclose(input_file);
		free(rom_content);
		emu_destroy(&emu);
		return 1;
	}
	fclose(input_file);

	for (size_t i = 0; i < file_size; i++) {
		emu_w8(&emu, rom_base + i, rom_content[i]);
	}
	free(rom_content);

	if (hdd_file != NULL &&
	    !devices_create_virt_machine(&emu, hdd_file)) {
		fprintf(stderr, "Unable to create a \"virt\" machine\n");
		emu_destroy(&emu);
		return 1;
	}

	while (emu.running) {
		cpu_execute(&emu);
	}

	bool reboot = emu.reboot;
	emu_destroy(&emu);
	if (reboot) {
		return main_advanced(argc, argv);
	} else {
		return 0;
	}
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

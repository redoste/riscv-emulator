#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "emulator.h"

void emu_create(emulator_t* emu, guest_paddr rom_base, size_t rom_size, guest_paddr ram_base, size_t ram_size) {
	uint8_t* rom_pool = malloc(rom_size);
	uint8_t* ram_pool = malloc(ram_size);
	assert(rom_pool != NULL && ram_pool != NULL);

	// TODO : check for overlap
	mem_region_t rom = {
		.pool = rom_pool,
		.base = rom_base,
		.size = rom_size,
	};
	mem_region_t ram = {
		.pool = ram_pool,
		.base = ram_base,
		.size = ram_size,
	};

	emu->rom = rom;
	emu->ram = ram;

	memset(&emu->cpu, 0, sizeof(emu->cpu));
	// NOTE : we assume the entry point is at the start of the ROM
	emu->cpu.pc = emu->rom.base;
}

void emu_destroy(emulator_t* emu) {
	free(emu->rom.pool);
	free(emu->ram.pool);
}

// TODO : support BE hosts

#define EMU_RX(SIZE, TYPE)                                                         \
	TYPE emu_r##SIZE(emulator_t* emu, guest_paddr addr) {                      \
		if (addr - emu->rom.base < emu->rom.size &&                        \
		    addr - emu->rom.base + sizeof(TYPE) <= emu->rom.size) {        \
			TYPE* value = (TYPE*)&emu->rom.pool[addr - emu->rom.base]; \
			return *value;                                             \
		} else if (addr - emu->ram.base < emu->ram.size &&                 \
			   addr - emu->ram.base + sizeof(TYPE) <= emu->ram.size) { \
			TYPE* value = (TYPE*)&emu->ram.pool[addr - emu->ram.base]; \
			return *value;                                             \
		}                                                                  \
                                                                                   \
		fprintf(stderr, "invalid r" #SIZE " at %016lx PC=%016lx\n",        \
			addr, emu->cpu.pc);                                        \
		abort();                                                           \
	}

#define EMU_WX(SIZE, TYPE)                                                             \
	void emu_w##SIZE(emulator_t* emu, guest_paddr addr, TYPE value) {              \
		if (addr - emu->rom.base < emu->rom.size &&                            \
		    addr - emu->rom.base + sizeof(TYPE) <= emu->rom.size) {            \
			TYPE* host_addr = (TYPE*)&emu->rom.pool[addr - emu->rom.base]; \
			*host_addr = value;                                            \
			return;                                                        \
		} else if (addr - emu->ram.base < emu->ram.size &&                     \
			   addr - emu->ram.base + sizeof(TYPE) <= emu->ram.size) {     \
			TYPE* host_addr = (TYPE*)&emu->ram.pool[addr - emu->ram.base]; \
			*host_addr = value;                                            \
			return;                                                        \
		}                                                                      \
                                                                                       \
		fprintf(stderr, "invalid w" #SIZE " at %016lx PC=%016lx\n",            \
			addr, emu->cpu.pc);                                            \
		abort();                                                               \
	}

EMU_RX(8, uint8_t)
EMU_RX(16, uint16_t)
EMU_RX(32, uint32_t)
EMU_RX(64, uint64_t)
EMU_WX(8, uint8_t)
EMU_WX(16, uint16_t)
EMU_WX(32, uint32_t)
EMU_WX(64, uint64_t)

void emu_ebreak(emulator_t* emu) {
	fprintf(stderr, "EBREAK PC=%016lx\n", emu->cpu.pc);
	for (size_t i = 0; i < REG_COUNT; i++) {
		fprintf(stderr, "x%2zu=%016lx ", i, emu->cpu.regs[i]);
		if ((i % 4) == 3) {
			fprintf(stderr, "\n");
		}
	}
}

void emu_ecall(emulator_t* emu) {
	emu_ebreak(emu);
}

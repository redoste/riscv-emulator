#ifndef EMULATOR_H
#define EMULATOR_H

#include <stddef.h>
#include <stdint.h>

#include "cpu.h"
#include "isa.h"

typedef struct mem_region_t {
	uint8_t* pool;
	guest_paddr base;
	size_t size;
} mem_region_t;

typedef struct emulator_t {
	cpu_t cpu;

	/* TODO : support more memory regions using a paging-like structure
	 *        this would also allow us to do MMIO
	 */
	mem_region_t rom;
	mem_region_t ram;
} emulator_t;

void emu_create(emulator_t* emu, guest_paddr rom_base, size_t rom_size, guest_paddr ram_base, size_t ram_size);
void emu_destroy(emulator_t* emu);

void emu_w8(emulator_t* emu, guest_paddr addr, uint8_t value);
void emu_w16(emulator_t* emu, guest_paddr addr, uint16_t value);
void emu_w32(emulator_t* emu, guest_paddr addr, uint32_t value);
void emu_w64(emulator_t* emu, guest_paddr addr, uint64_t value);

uint8_t emu_r8(emulator_t* emu, guest_paddr addr);
uint16_t emu_r16(emulator_t* emu, guest_paddr addr);
uint32_t emu_r32(emulator_t* emu, guest_paddr addr);
uint64_t emu_r64(emulator_t* emu, guest_paddr addr);

void emu_ebreak(emulator_t* emu);
void emu_ecall(emulator_t* emu);

#endif

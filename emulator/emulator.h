#ifndef EMULATOR_H
#define EMULATOR_H

#include <stddef.h>
#include <stdint.h>

#include "cpu.h"
#include "emulator_sdl.h"
#include "isa.h"

/* mem_region_t : structure representing a single guest memory region
 *                for now memory regions are juste a piece of host memory but if we use MMIO in
 *                the future, it's possible that they become a list of function pointers
 *                responsible for handeling the I/O
 */
typedef struct mem_region_t {
	uint8_t* pool;
	guest_paddr base;
	size_t size;
} mem_region_t;

/* emulator_t : structure storing the emulator state
 */
typedef struct emulator_t {
	cpu_t cpu;

	/* TODO : support more memory regions using a paging-like structure
	 *        this would also allow us to do MMIO
	 */
	mem_region_t rom;
	mem_region_t ram;

#ifdef RISCV_EMULATOR_SDL_SUPPORT
	emu_sdl_data_t sdl_data;
#endif
} emulator_t;

/* emu_create : create an emulator
 *     emulator_t* emu               : pointer to the emulator_t struct to initialize
 *     guest_paddr rom_base          : base guest phyisical address of the ROM
 *     size_t rom_size               : size of the ROM
 *     guest_paddr ram_base          : base guest phyisical address of the RAM
 *     size_t ram_size               : size of the RAM
 *     size_t instruction_cache_bits : number of significant bits for the instruction cache
 */
void emu_create(emulator_t* emu, guest_paddr rom_base, size_t rom_size, guest_paddr ram_base, size_t ram_size, size_t instruction_cache_bits);

/* emu_destroy : destory an emulator and free its associated ressources
 *     emulator_t* emu : pointer to the emulator_t struct to destroy
 */
void emu_destroy(emulator_t* emu);

/* emu_wx : write a x bits value to the guest memory
 *     emulator_t* emu  : pointer to the emulator
 *     guest_paddr addr : guest physicial address to write to
 *     uintx_t value    : value to write to the guest memory
 */
void emu_w8(emulator_t* emu, guest_paddr addr, uint8_t value);
void emu_w16(emulator_t* emu, guest_paddr addr, uint16_t value);
void emu_w32(emulator_t* emu, guest_paddr addr, uint32_t value);
void emu_w64(emulator_t* emu, guest_paddr addr, uint64_t value);

/* emu_rx : read a x bits value from the guest memory
 *          returns the value read
 *     emulator_t* emu  : pointer to the emulator
 *     guest_paddr addr : guest physicial address to read from
 */
uint8_t emu_r8(emulator_t* emu, guest_paddr addr);
uint16_t emu_r16(emulator_t* emu, guest_paddr addr);
uint32_t emu_r32(emulator_t* emu, guest_paddr addr);
uint64_t emu_r64(emulator_t* emu, guest_paddr addr);

/* emu_ebreak : handle the ebreak instruction
 *     emulator_t* emu : pointer to the emulator
 */
void emu_ebreak(emulator_t* emu);

/* emu_ecall : handle the ecall instruction
 *             a few "emucalls" are defined and are used by the guest to communicate with the emulator
 *     emulator_t* emu : pointer to the emulator
 */
void emu_ecall(emulator_t* emu);

#endif

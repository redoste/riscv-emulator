#ifndef EMULATOR_H
#define EMULATOR_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "cpu.h"
#include "devices.h"
#include "emulator_sdl.h"
#include "isa.h"
#include "mmu_paging_guest_to_host.h"

/* emulator_t : structure storing the emulator state
 */
typedef struct emulator_t {
	cpu_t cpu;

	mmu_pg2h_pte pg2h_paging_table;
	mmu_pg2h_tlb_entry_t* pg2h_tlb;
	guest_paddr pg2h_tlb_mask;

	device_mmio_t* mmio_devices;
	size_t mmio_devices_len;
	size_t mmio_devices_capacity;

#ifdef RISCV_EMULATOR_SDL_SUPPORT
	emu_sdl_data_t sdl_data;
#endif
} emulator_t;

/* emu_create : create an emulator
 *     emulator_t* emu               : pointer to the emulator_t struct to initialize
 *     guest_reg pc                  : initial value for the program counter
 *     size_t cache_bits             : number of significant bits for the different caches
 *     bool dynarec_enabled          : enable dynamic recompilation
 *     bool user_only_mode           : enable user only mode
 */
void emu_create(emulator_t* emu, guest_reg pc, size_t cache_bits, bool dynarec_enabled, bool user_only_mode);

/* emu_destroy : destroy an emulator and free its associated ressources
 *     emulator_t* emu : pointer to the emulator_t struct to destroy
 */
void emu_destroy(emulator_t* emu);

/* emu_map_memory : map a chunk of mmaped memory to the guest
 *                  returns true if the memory was successfully allocated and mapped
 *                  returns false otherwise
 *     emulator_t* emu  : pointer to the emulator
 *     guest_paddr base : base guest physical address for the newly allocated memory
 *     size_t size      : size of the memory chunk to mmap
 */
bool emu_map_memory(emulator_t* emu, guest_paddr base, size_t size);

/* emu_add_mmio_device : map and add a MMIO device to the guest
 *                       returns true if the device was successfully attached
 *                       returns false otherwise
 *     emulator_t* emu       : pointer to the emulator
 *     guest_paddr base      : base guest physical address for the new device
 *     device_mmio_t* device : pointer to the MMIO device handlers
 */
bool emu_add_mmio_device(emulator_t* emu, guest_paddr base, const device_mmio_t* device);

/* emu_wx : write a x bits value to the guest memory
 *          returns true if a cache entry was invalidated in the process
 *          returns false otherwise
 *     emulator_t* emu   : pointer to the emulator
 *     guest_vaddr vaddr : guest virtual address to write to
 *     uintx_t value     : value to write to the guest memory
 */
bool emu_w8(emulator_t* emu, guest_vaddr vaddr, uint8_t value);
bool emu_w16(emulator_t* emu, guest_vaddr vaddr, uint16_t value);
bool emu_w32(emulator_t* emu, guest_vaddr vaddr, uint32_t value);
bool emu_w64(emulator_t* emu, guest_vaddr vaddr, uint64_t value);

/* emu_rx : read a x bits value from the guest memory
 *          returns the value read
 *     emulator_t* emu   : pointer to the emulator
 *     guest_vaddr vaddr : guest virtual address to read from
 */
uint8_t emu_r8(emulator_t* emu, guest_vaddr vaddr);
uint16_t emu_r16(emulator_t* emu, guest_vaddr vaddr);
uint32_t emu_r32(emulator_t* emu, guest_vaddr vaddr);
uint64_t emu_r64(emulator_t* emu, guest_vaddr vaddr);

/* emu_r32_ins : read a 32 bits instruction from the guest memory
 *               reading instructions doesn't use emu_r32 because exception codes are
 *               different and the caller might want to ignore the exception
 *               returns the value read
 *     emulator_t* emu           : pointer to the emulator
 *     guest_vaddr vaddr         : guest virtual address to read from
 *     uint8_t* exception_code   : exception code if an exception should occur during the read
 *                                 (uint8_t)-1 otherwise
 *     guest_reg* exception_tval : exception tval if an exception should occur during the read
 */
uint32_t emu_r32_ins(emulator_t* emu, guest_vaddr vaddr, uint8_t* exception_code, guest_reg* exception_tval);

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

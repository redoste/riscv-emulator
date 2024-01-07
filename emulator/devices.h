#ifndef DEVICES_H
#define DEVICES_H

#include <stdint.h>

#include "isa.h"

// NOTE : forward declaration to deal with a cyclic dependency with emulator.h
typedef struct emulator_t emulator_t;

/* device_free_handler_t : typedef for the MMIO device free handler
 *                         it will be called by the emulator in emu_destroy and should free
 *                         the device_data
 */
typedef void (*device_free_handler_t)(emulator_t*, void*);

/* device_update_handler_t : typedef for the MMIO device update handler
 *                           it will be called regularly by the emulator to let the device
 *                           update its state
 */
typedef void (*device_update_handler_t)(emulator_t*, void*);

/* device_{r,w}x_handler_t : typedef for the MMIO device R/W handlers
 */
typedef uint8_t (*device_r8_handler_t)(emulator_t*, void*, guest_paddr);
typedef uint16_t (*device_r16_handler_t)(emulator_t*, void*, guest_paddr);
typedef uint32_t (*device_r32_handler_t)(emulator_t*, void*, guest_paddr);
typedef uint64_t (*device_r64_handler_t)(emulator_t*, void*, guest_paddr);
typedef void (*device_w8_handler_t)(emulator_t*, void*, guest_paddr, uint8_t);
typedef void (*device_w16_handler_t)(emulator_t*, void*, guest_paddr, uint16_t);
typedef void (*device_w32_handler_t)(emulator_t*, void*, guest_paddr, uint32_t);
typedef void (*device_w64_handler_t)(emulator_t*, void*, guest_paddr, uint64_t);

/* device_mmio_t : structure storing the handlers for a MMIO device
 */
typedef struct device_mmio_t {
	void* device_data;

	device_free_handler_t free_handler;
	device_update_handler_t update_handler;

	device_r8_handler_t r8_handler;
	device_r16_handler_t r16_handler;
	device_r32_handler_t r32_handler;
	device_r64_handler_t r64_handler;
	device_w8_handler_t w8_handler;
	device_w16_handler_t w16_handler;
	device_w32_handler_t w32_handler;
	device_w64_handler_t w64_handler;
} device_mmio_t;

#endif

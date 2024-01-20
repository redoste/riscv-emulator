#include <stdbool.h>
#include <stdint.h>

#include "device_syscon.h"
#include "emulator.h"
#include "isa.h"

#define SYSCON_POWEROFF_MAGIC 0x5555
#define SYSCON_REBOOT_MAGIC   0x7777

static void syscon_free(emulator_t* emu, void* device_data) {
	(void)emu;

	free(device_data);
}

static uint8_t syscon_r8(emulator_t* emu, void* device_data, guest_paddr addr) {
	guest_paddr* syscon_base = (guest_paddr*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, *syscon_base + addr);
	return 0;
}

static uint16_t syscon_r16(emulator_t* emu, void* device_data, guest_paddr addr) {
	guest_paddr* syscon_base = (guest_paddr*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, *syscon_base + addr);
	return 0;
}

static uint32_t syscon_r32(emulator_t* emu, void* device_data, guest_paddr addr) {
	guest_paddr* syscon_base = (guest_paddr*)device_data;
	if (addr != 0) {
		cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, *syscon_base + addr);
	}
	return 0;
}

static uint64_t syscon_r64(emulator_t* emu, void* device_data, guest_paddr addr) {
	guest_paddr* syscon_base = (guest_paddr*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, *syscon_base + addr);
	return 0;
}

static void syscon_w8(emulator_t* emu, void* device_data, guest_paddr addr, uint8_t value) {
	(void)value;

	guest_paddr* syscon_base = (guest_paddr*)device_data;
	cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, *syscon_base + addr);
}

static void syscon_w16(emulator_t* emu, void* device_data, guest_paddr addr, uint16_t value) {
	(void)value;

	guest_paddr* syscon_base = (guest_paddr*)device_data;
	cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, *syscon_base + addr);
}

static void syscon_w32(emulator_t* emu, void* device_data, guest_paddr addr, uint32_t value) {
	(void)value;

	if (addr == 0 && value == SYSCON_POWEROFF_MAGIC) {
		emu->running = false;
		emu->reboot = false;
	} else if (addr == 0 && value == SYSCON_REBOOT_MAGIC) {
		emu->running = false;
		emu->reboot = true;
	} else {
		guest_paddr* syscon_base = (guest_paddr*)device_data;
		cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, *syscon_base + addr);
	}
}

static void syscon_w64(emulator_t* emu, void* device_data, guest_paddr addr, uint64_t value) {
	(void)value;

	guest_paddr* syscon_base = (guest_paddr*)device_data;
	cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, *syscon_base + addr);
}

bool syscon_create(emulator_t* emu, guest_paddr base) {
	guest_paddr* syscon_base = malloc(sizeof(guest_paddr));
	assert(syscon_base != NULL);
	*syscon_base = base;

	device_mmio_t device = {
		syscon_base,
		syscon_free,
		NULL,
		syscon_r8,
		syscon_r16,
		syscon_r32,
		syscon_r64,
		syscon_w8,
		syscon_w16,
		syscon_w32,
		syscon_w64,
	};

	if (!emu_add_mmio_device(emu, base, 0x1000, &device)) {
		free(syscon_base);
		return false;
	}
	return true;
}

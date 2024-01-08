#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>

#include "device_clint.h"
#include "emulator.h"
#include "isa.h"

#define CLINT_TICK_PER_NS 100  // The CLINT is running at 10 MHz

#define CLINT_MSWI_BASE     0x0000
#define CLINT_MTIMECMP_BASE 0x4000
#define CLINT_MTIME_BASE    0xbff8

typedef struct clint_t {
	uint64_t monotonic_base;
	uint64_t mtimecmp;
	guest_paddr base;
} clint_t;

static inline uint64_t clint_get_monotonic_ticks(void) {
	struct timespec tp;
	clock_gettime(CLOCK_MONOTONIC, &tp);
	return (tp.tv_sec * (1000000000 / CLINT_TICK_PER_NS)) +
	       (tp.tv_nsec / CLINT_TICK_PER_NS);
}

uint64_t clint_get_mtime(const clint_t* clint) {
	return clint_get_monotonic_ticks() - clint->monotonic_base;
}

static inline void clint_update_mtip(emulator_t* emu, clint_t* clint) {
	if (clint->mtimecmp <= clint_get_mtime(clint)) {
		emu->cpu.csrs.mip |= (1 << 7);  // MTIP
	} else {
		emu->cpu.csrs.mip &= ~(1 << 7);  // MTIP
	}
}

static void clint_free(emulator_t* emu, void* device_data) {
	(void)emu;

	clint_t* clint = (clint_t*)device_data;
	free(clint);
}

static uint8_t clint_r8(emulator_t* emu, void* device_data, guest_paddr addr) {
	clint_t* clint = (clint_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, clint->base + addr);
	return 0;
}

static uint16_t clint_r16(emulator_t* emu, void* device_data, guest_paddr addr) {
	clint_t* clint = (clint_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, clint->base + addr);
	return 0;
}

static uint32_t clint_r32(emulator_t* emu, void* device_data, guest_paddr addr) {
	clint_t* clint = (clint_t*)device_data;
	if (addr == CLINT_MSWI_BASE) {
		return (emu->cpu.csrs.mip >> 3) & 1;  // MSIP
	} else {
		cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, clint->base + addr);
		return 0;
	}
}

static uint64_t clint_r64(emulator_t* emu, void* device_data, guest_paddr addr) {
	clint_t* clint = (clint_t*)device_data;
	if (addr == CLINT_MTIMECMP_BASE) {
		return clint->mtimecmp;
	} else if (addr == CLINT_MTIME_BASE) {
		return clint_get_mtime(clint);
	} else {
		cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, clint->base + addr);
		return 0;
	}
}

static void clint_w8(emulator_t* emu, void* device_data, guest_paddr addr, uint8_t value) {
	(void)value;

	clint_t* clint = (clint_t*)device_data;
	cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, clint->base + addr);
	return;
}

static void clint_w16(emulator_t* emu, void* device_data, guest_paddr addr, uint16_t value) {
	(void)value;

	clint_t* clint = (clint_t*)device_data;
	cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, clint->base + addr);
	return;
}

static void clint_w32(emulator_t* emu, void* device_data, guest_paddr addr, uint32_t value) {
	clint_t* clint = (clint_t*)device_data;
	if (addr == CLINT_MSWI_BASE) {
		emu->cpu.csrs.mip = (emu->cpu.csrs.mip & ~(1 << 3)) |  // MSIP
				    ((value & 1) << 3);
	} else {
		cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, clint->base + addr);
	}
}

static void clint_w64(emulator_t* emu, void* device_data, guest_paddr addr, uint64_t value) {
	clint_t* clint = (clint_t*)device_data;
	if (addr == CLINT_MTIMECMP_BASE) {
		clint->mtimecmp = value;
		clint_update_mtip(emu, clint);
	} else if (addr == CLINT_MTIME_BASE) {
		clint->monotonic_base = clint_get_monotonic_ticks() + value;
		clint_update_mtip(emu, clint);
	} else {
		cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, clint->base + addr);
	}
}

static void clint_update(emulator_t* emu, void* device_data) {
	clint_t* clint = (clint_t*)device_data;
	clint_update_mtip(emu, clint);
}

bool clint_create(emulator_t* emu, guest_paddr base) {
	clint_t* clint = malloc(sizeof(clint_t));
	clint->monotonic_base = clint_get_monotonic_ticks();
	clint->mtimecmp = (uint64_t)-1;
	clint->base = base;

	device_mmio_t device = {
		clint,
		clint_free,
		clint_update,
		clint_r8,
		clint_r16,
		clint_r32,
		clint_r64,
		clint_w8,
		clint_w16,
		clint_w32,
		clint_w64,
	};

	if (!emu_add_mmio_device(emu, base, 0x10000, &device)) {
		free(clint);
		return false;
	}
	return true;
}

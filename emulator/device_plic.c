#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "device_plic.h"
#include "emulator.h"
#include "isa.h"

#define PLIC_N_INT 32

#define PLIC_INT_SOURCE_PRIORITY_BASE      0x000000
#define PLIC_INT_SOURCE_PRIORITY_END       (PLIC_INT_SOURCE_PRIORITY_BASE + (PLIC_N_INT * sizeof(uint32_t)))
#define PLIC_INT_PENDING_BASE              0x001000
#define PLIC_INT_PENDING_END               (PLIC_INT_PENDING_BASE + (PLIC_N_INT / 8))
#define PLIC_INT_ENABLE_CTX_0_BASE         0x002000
#define PLIC_INT_ENABLE_CTX_0_END          (PLIC_INT_ENABLE_CTX_0_BASE + (PLIC_N_INT / 8))
#define PLIC_INT_ENABLE_CTX_1_BASE         0x002080
#define PLIC_INT_ENABLE_CTX_1_END          (PLIC_INT_ENABLE_CTX_1_BASE + (PLIC_N_INT / 8))
#define PLIC_PRIORITY_THRESHOLD_CTX_0_BASE 0x200000
#define PLIC_CLAIM_CTX_0_BASE              0x200004
#define PLIC_PRIORITY_THRESHOLD_CTX_1_BASE 0x201000
#define PLIC_CLAIM_CTX_1_BASE              0x201004

typedef struct plic_t {
	guest_paddr base;

	uint32_t int_source_priority[PLIC_N_INT];
	uint32_t int_pending[PLIC_N_INT / 32];

	// The context 0 is delivered to M-mode via MEIP
	uint32_t enable_ctx_0[PLIC_N_INT / 32];
	uint32_t priority_threshold_ctx_0;
	uint32_t claim_ctx_0;

	// The context 1 is delivered to S-mode via SEIP
	uint32_t enable_ctx_1[PLIC_N_INT / 32];
	uint32_t priority_threshold_ctx_1;
	uint32_t claim_ctx_1;
} plic_t;

static void plic_free(emulator_t* emu, void* device_data) {
	(void)emu;

	plic_t* plic = (plic_t*)device_data;
	free(plic);
}

static uint8_t plic_r8(emulator_t* emu, void* device_data, guest_paddr addr) {
	plic_t* plic = (plic_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, plic->base + addr);
	return 0;
}

static uint16_t plic_r16(emulator_t* emu, void* device_data, guest_paddr addr) {
	plic_t* plic = (plic_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, plic->base + addr);
	return 0;
}

static uint32_t plic_r32(emulator_t* emu, void* device_data, guest_paddr addr) {
	plic_t* plic = (plic_t*)device_data;

	if (addr < PLIC_INT_SOURCE_PRIORITY_END) {
		return plic->int_source_priority[(addr - PLIC_INT_SOURCE_PRIORITY_BASE) / sizeof(uint32_t)];
	} else if (addr >= PLIC_INT_PENDING_BASE && addr < PLIC_INT_PENDING_END) {
		return plic->int_pending[(addr - PLIC_INT_PENDING_BASE) / sizeof(uint32_t)];
	} else if (addr >= PLIC_INT_ENABLE_CTX_0_BASE && addr < PLIC_INT_ENABLE_CTX_0_END) {
		return plic->enable_ctx_0[(addr - PLIC_INT_ENABLE_CTX_0_BASE) / sizeof(uint32_t)];
	} else if (addr >= PLIC_INT_ENABLE_CTX_1_BASE && addr < PLIC_INT_ENABLE_CTX_1_END) {
		return plic->enable_ctx_1[(addr - PLIC_INT_ENABLE_CTX_1_BASE) / sizeof(uint32_t)];
	} else if (addr == PLIC_PRIORITY_THRESHOLD_CTX_0_BASE) {
		return plic->priority_threshold_ctx_0;
	} else if (addr == PLIC_CLAIM_CTX_0_BASE) {
		return plic->claim_ctx_0;
	} else if (addr == PLIC_PRIORITY_THRESHOLD_CTX_1_BASE) {
		return plic->priority_threshold_ctx_1;
	} else if (addr == PLIC_CLAIM_CTX_1_BASE) {
		return plic->claim_ctx_1;
	} else {
		cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, plic->base + addr);
		return 0;
	}
}

static uint64_t plic_r64(emulator_t* emu, void* device_data, guest_paddr addr) {
	plic_t* plic = (plic_t*)device_data;
	cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, plic->base + addr);
	return 0;
}

static void plic_w8(emulator_t* emu, void* device_data, guest_paddr addr, uint8_t value) {
	(void)value;

	plic_t* plic = (plic_t*)device_data;
	cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, plic->base + addr);
}

static void plic_w16(emulator_t* emu, void* device_data, guest_paddr addr, uint16_t value) {
	(void)value;

	plic_t* plic = (plic_t*)device_data;
	cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, plic->base + addr);
}

static void plic_w32(emulator_t* emu, void* device_data, guest_paddr addr, uint32_t value) {
	plic_t* plic = (plic_t*)device_data;

	if (addr < PLIC_INT_SOURCE_PRIORITY_END) {
		plic->int_source_priority[(addr - PLIC_INT_SOURCE_PRIORITY_BASE) / sizeof(uint32_t)] = value;
	} else if (addr >= PLIC_INT_PENDING_BASE && addr < PLIC_INT_PENDING_END) {
		plic->int_pending[(addr - PLIC_INT_PENDING_BASE) / sizeof(uint32_t)] = value;
	} else if (addr >= PLIC_INT_ENABLE_CTX_0_BASE && addr < PLIC_INT_ENABLE_CTX_0_END) {
		plic->enable_ctx_0[(addr - PLIC_INT_ENABLE_CTX_0_BASE) / sizeof(uint32_t)] = value;
	} else if (addr >= PLIC_INT_ENABLE_CTX_1_BASE && addr < PLIC_INT_ENABLE_CTX_1_END) {
		plic->enable_ctx_1[(addr - PLIC_INT_ENABLE_CTX_1_BASE) / sizeof(uint32_t)] = value;
	} else if (addr == PLIC_PRIORITY_THRESHOLD_CTX_0_BASE) {
		plic->priority_threshold_ctx_0 = value;
	} else if (addr == PLIC_CLAIM_CTX_0_BASE) {
		if (value == plic->claim_ctx_0) {
			assert(value > 0 && value < PLIC_N_INT);
			plic->int_pending[value / 32] &= ~(1 << (value & 0x1f));
			plic->claim_ctx_0 = 0;
			emu->cpu.csrs.mip &= ~(1 << 11);  // MEIP
		}
	} else if (addr == PLIC_PRIORITY_THRESHOLD_CTX_1_BASE) {
		plic->priority_threshold_ctx_1 = value;
	} else if (addr == PLIC_CLAIM_CTX_1_BASE) {
		if (value == plic->claim_ctx_1) {
			assert(value > 0 && value < PLIC_N_INT);
			plic->int_pending[value / 32] &= ~(1 << (value & 0x1f));
			plic->claim_ctx_1 = 0;
			emu->cpu.csrs.mip &= ~(1 << 9);  // SEIP
		}
	} else {
		cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, plic->base + addr);
	}
}

static void plic_w64(emulator_t* emu, void* device_data, guest_paddr addr, uint64_t value) {
	(void)value;

	plic_t* plic = (plic_t*)device_data;
	cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, plic->base + addr);
}

static void plic_update(emulator_t* emu, void* device_data) {
	plic_t* plic = (plic_t*)device_data;

	// TODO : deliver interrupts in the order of priority
	for (size_t i = 1; i < PLIC_N_INT; i++) {
		uint32_t int_priority = plic->int_source_priority[i];
		size_t u32_index = i / 32;
		size_t bit_index = i & 0x1f;

		if ((plic->int_pending[u32_index] >> bit_index) & 1) {
			if (((plic->enable_ctx_0[u32_index] >> bit_index) & 1) &&
			    plic->priority_threshold_ctx_0 < int_priority &&
			    plic->claim_ctx_0 == 0) {
				plic->claim_ctx_0 = i;
				emu->cpu.csrs.mip |= (1 << 11);  // MEIP
			}

			if (((plic->enable_ctx_1[u32_index] >> bit_index) & 1) &&
			    plic->priority_threshold_ctx_1 < int_priority &&
			    plic->claim_ctx_1 == 0) {
				plic->claim_ctx_1 = i;
				emu->cpu.csrs.mip |= (1 << 9);  // SEIP
			}
		}
	}
}

void plic_throw_interrupt(emulator_t* emu, size_t int_number) {
	if (emu->plic == NULL || int_number == 0 || int_number >= PLIC_N_INT) {
		return;
	}

	plic_t* plic = emu->plic;
	plic->int_pending[int_number / 32] |= (1 << (int_number & 0x1f));
}

bool plic_create(emulator_t* emu, guest_paddr base) {
	// Only a single PLIC can be attached to the emulator
	if (emu->plic != NULL) {
		return false;
	}

	plic_t* plic = malloc(sizeof(plic_t));
	assert(plic != NULL);
	memset(plic, 0, sizeof(plic_t));
	plic->base = base;

	emu->plic = plic;

	device_mmio_t device = {
		plic,
		plic_free,
		plic_update,
		plic_r8,
		plic_r16,
		plic_r32,
		plic_r64,
		plic_w8,
		plic_w16,
		plic_w32,
		plic_w64,
	};

	if (!emu_add_mmio_device(emu, base, 0x4000000, &device)) {
		free(plic);
		return false;
	}
	return true;
}

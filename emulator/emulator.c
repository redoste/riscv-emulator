#include <assert.h>
#include <endian.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "cpu.h"
#include "devices.h"
#include "dynarec_x86_64.h"
#include "emulator.h"
#include "emulator_sdl.h"
#include "mmu_paging_guest_to_guest.h"
#include "mmu_paging_guest_to_host.h"

void emu_create(emulator_t* emu, guest_reg pc, size_t cache_bits, bool dynarec_enabled, bool user_only_mode) {
	emu->pg2h_paging_table = 0;

	memset(&emu->cpu, 0, sizeof(emu->cpu));
	emu->cpu.pc = pc;
	emu->cpu.priv_mode = user_only_mode ? UO_MODE : M_MODE;
	emu->cpu.dynarec_enabled = dynarec_enabled;

	if (cache_bits > 24) {
		fprintf(stderr, "The number of significant bits for the caches is over 24 bits\n");
		abort();
	}

	const guest_paddr caches_mask = (1ull << cache_bits) - 1;

	size_t instruction_cache_size;
	if (dynarec_enabled) {
#ifdef RISCV_EMULATOR_DYNAREC_X86_64_SUPPORT
		instruction_cache_size = (1ull << cache_bits) *
					 sizeof(emu->cpu.instruction_cache.as_dr_ins[0]);
#else
		fprintf(stderr, "Dynarec support isn't enabled\n");
		abort();
#endif
	} else {
		instruction_cache_size = (1ull << cache_bits) *
					 sizeof(emu->cpu.instruction_cache.as_cached_ins[0]);
	}
	emu->cpu.instruction_cache.as_ptr = malloc(instruction_cache_size);
	emu->cpu.instruction_cache_mask = caches_mask;
	assert(emu->cpu.instruction_cache.as_ptr != NULL);
	memset(emu->cpu.instruction_cache.as_ptr, 0, instruction_cache_size);

	const size_t pg2h_tlb_size = (1ull << cache_bits) * sizeof(emu->pg2h_tlb[0]);
	emu->pg2h_tlb = malloc(pg2h_tlb_size);
	emu->pg2h_tlb_mask = caches_mask;
	assert(emu->pg2h_tlb != NULL);
	memset(emu->pg2h_tlb, 0, pg2h_tlb_size);

	emu->mmio_devices = NULL;
	emu->mmio_devices_capacity = emu->mmio_devices_len = 0;

#ifdef RISCV_EMULATOR_SDL_SUPPORT
	memset(&emu->sdl_data, 0, sizeof(emu->sdl_data));
#endif
}

void emu_destroy(emulator_t* emu) {
	for (size_t i = 0; i < emu->mmio_devices_len; i++) {
		if (emu->mmio_devices[i].free_handler != NULL) {
			emu->mmio_devices[i].free_handler(emu, emu->mmio_devices[i].device_data);
		}
	}
	free(emu->mmio_devices);

	mmu_pg2h_free(emu);
#ifdef RISCV_EMULATOR_DYNAREC_X86_64_SUPPORT
	if (emu->cpu.dynarec_enabled) {
		dr_free(emu);
	}
#endif
	free(emu->cpu.instruction_cache.as_ptr);
	free(emu->pg2h_tlb);
#ifdef RISCV_EMULATOR_SDL_SUPPORT
	emu_sdl_destory(emu);
#endif
}

bool emu_map_memory(emulator_t* emu, guest_paddr base, size_t size) {
	if ((base & MMU_PG2H_OFFSET_MASK) != 0 ||
	    (size & MMU_PG2H_OFFSET_MASK) != 0) {
		return false;
	}

	uint8_t* pool = mmap(NULL, size, PROT_READ | PROT_WRITE,
			     MAP_PRIVATE | MAP_ANONYMOUS, 0, 0);
	if (pool == MAP_FAILED) {
		return false;
	}

	for (size_t i = 0; i < size / MMU_PG2H_PAGE_SIZE; i++) {
		if (!mmu_pg2h_map(emu, base + (i * MMU_PG2H_PAGE_SIZE), pool + (i * MMU_PG2H_PAGE_SIZE))) {
			for (size_t j = 0; j < i; j++) {
				mmu_pg2h_unmap(emu, base + (j * MMU_PG2H_PAGE_SIZE));
			}
			munmap(pool, size);
			return false;
		}
	}

	return true;
}

bool emu_add_mmio_device(emulator_t* emu, guest_paddr base, size_t size, const device_mmio_t* device) {
	if ((base & MMU_PG2H_OFFSET_MASK) != 0 ||
	    (size & MMU_PG2H_OFFSET_MASK) != 0) {
		return false;
	}

	if (emu->mmio_devices_capacity == 0) {
		emu->mmio_devices_capacity = 16;
		emu->mmio_devices = malloc(sizeof(device_mmio_t) * emu->mmio_devices_capacity);
		assert(emu->mmio_devices != NULL);
	} else if (emu->mmio_devices_len == emu->mmio_devices_capacity) {
		emu->mmio_devices_capacity *= 2;
		emu->mmio_devices = realloc(emu->mmio_devices, sizeof(device_mmio_t) * emu->mmio_devices_capacity);
		assert(emu->mmio_devices != NULL);
	}

	emu->mmio_devices[emu->mmio_devices_len] = *device;
	for (size_t i = 0; i < size / MMU_PG2H_PAGE_SIZE; i++) {
		if (!mmu_pg2h_map_mmio(emu, base + (i * MMU_PG2H_PAGE_SIZE), i, emu->mmio_devices_len)) {
			for (size_t j = 0; j < i; j++) {
				mmu_pg2h_unmap(emu, base + (j * MMU_PG2H_PAGE_SIZE));
			}
			return false;
		}
	}
	emu->mmio_devices_len++;
	return true;
}

void emu_update_mmio_devices(emulator_t* emu) {
	for (size_t i = 0; i < emu->mmio_devices_len; i++) {
		if (emu->mmio_devices[i].update_handler != NULL) {
			emu->mmio_devices[i].update_handler(emu, emu->mmio_devices[i].device_data);
		}
	}
}

#define le8toh(x) (x)
#define htole8(x) (x)

#define EMU_RX_MISALIGNED(SIZE, TYPE)                                                      \
	static inline TYPE emu_r##SIZE##_misaligned(emulator_t* emu, guest_vaddr vaddr) {  \
		TYPE value = 0;                                                            \
		for (size_t i = 0; i < sizeof(TYPE) && !emu->cpu.exception_pending; i++) { \
			value |= (TYPE)emu_r8(emu, vaddr + i) << (i * 8);                  \
		}                                                                          \
		return value;                                                              \
	}

#define EMU_RX(SIZE, TYPE)                                                                                                      \
	TYPE emu_r##SIZE(emulator_t* emu, guest_vaddr vaddr) {                                                                  \
		size_t offset = vaddr & MMU_PG2H_OFFSET_MASK;                                                                   \
		if ((offset & (sizeof(TYPE) - 1)) != 0) {                                                                       \
			return emu_r##SIZE##_misaligned(emu, vaddr);                                                            \
		}                                                                                                               \
		/* Read across page boudaries should be handled by the misaligned case */                                       \
		assert(offset + sizeof(TYPE) <= MMU_PG2H_PAGE_SIZE);                                                            \
                                                                                                                                \
		guest_paddr paddr;                                                                                              \
		if (mmu_vg2pg_should_translate(emu, true)) {                                                                    \
			if (!mmu_vg2pg_translate(emu, MMU_VG2PG_ACCESS_READ,                                                    \
						 vaddr, &paddr)) {                                                              \
				cpu_throw_exception(emu, EXC_LOAD_PAGE_FAULT, vaddr);                                           \
				return 0;                                                                                       \
			}                                                                                                       \
		} else {                                                                                                        \
			paddr = vaddr;                                                                                          \
		}                                                                                                               \
                                                                                                                                \
		mmu_pg2h_pte pte;                                                                                               \
		if (!mmu_pg2h_get_pte(emu, paddr, &pte)) {                                                                      \
			cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, paddr);                                                 \
			return 0;                                                                                               \
		}                                                                                                               \
                                                                                                                                \
		if (pte & MMU_PG2H_PTE_TYPE_MMIO) {                                                                             \
			size_t device_index = (pte >> MMU_PG2H_PTE_DEVICE_SHIFT) & MMU_PG2H_PTE_DEVICE_MASK;                    \
			size_t page_index = (pte >> MMU_PG2H_PTE_DEVICE_PAGE_SHIFT) & MMU_PG2H_PTE_DEVICE_PAGE_MASK;            \
			device_mmio_t* device = &emu->mmio_devices[device_index];                                               \
			return device->r##SIZE##_handler(emu, device->device_data, (page_index * MMU_PG2H_PAGE_SIZE) + offset); \
		} else {                                                                                                        \
			uint8_t* pool = (uint8_t*)(pte & MMU_PG2H_PAGE_MASK);                                                   \
			TYPE* value = (TYPE*)&pool[offset];                                                                     \
			return le##SIZE##toh(*value);                                                                           \
		}                                                                                                               \
	}

#define EMU_WX_MISALIGNED(SIZE, TYPE)                                                                 \
	static inline bool emu_w##SIZE##_misaligned(emulator_t* emu, guest_vaddr vaddr, TYPE value) { \
		bool ret = false;                                                                     \
		for (size_t i = 0; i < sizeof(TYPE) && !emu->cpu.exception_pending; i++) {            \
			ret |= emu_w8(emu, vaddr + i, value & 0xff);                                  \
			value >>= 8;                                                                  \
		}                                                                                     \
		return ret;                                                                           \
	}

#define EMU_WX(SIZE, TYPE)                                                                                                      \
	bool emu_w##SIZE(emulator_t* emu, guest_vaddr vaddr, TYPE value) {                                                      \
		size_t offset = vaddr & MMU_PG2H_OFFSET_MASK;                                                                   \
		if ((offset & (sizeof(TYPE) - 1)) != 0) {                                                                       \
			return emu_w##SIZE##_misaligned(emu, vaddr, value);                                                     \
		}                                                                                                               \
		/* Write across page boudaries should be handled by the misaligned case */                                      \
		assert(offset + sizeof(TYPE) <= MMU_PG2H_PAGE_SIZE);                                                            \
                                                                                                                                \
		guest_paddr paddr;                                                                                              \
		if (mmu_vg2pg_should_translate(emu, true)) {                                                                    \
			if (!mmu_vg2pg_translate(emu, MMU_VG2PG_ACCESS_WRITE,                                                   \
						 vaddr, &paddr)) {                                                              \
				cpu_throw_exception(emu, EXC_STORE_PAGE_FAULT, vaddr);                                          \
				return false;                                                                                   \
			}                                                                                                       \
		} else {                                                                                                        \
			paddr = vaddr;                                                                                          \
		}                                                                                                               \
                                                                                                                                \
		bool ret = cpu_invalidate_instruction_cache(emu, vaddr);                                                        \
                                                                                                                                \
		mmu_pg2h_pte pte;                                                                                               \
		if (!mmu_pg2h_get_pte(emu, paddr, &pte)) {                                                                      \
			cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, paddr);                                                \
			return ret;                                                                                             \
		}                                                                                                               \
                                                                                                                                \
		if (pte & MMU_PG2H_PTE_TYPE_MMIO) {                                                                             \
			size_t device_index = (pte >> MMU_PG2H_PTE_DEVICE_SHIFT) & MMU_PG2H_PTE_DEVICE_MASK;                    \
			size_t page_index = (pte >> MMU_PG2H_PTE_DEVICE_PAGE_SHIFT) & MMU_PG2H_PTE_DEVICE_PAGE_MASK;            \
			device_mmio_t* device = &emu->mmio_devices[device_index];                                               \
			device->w##SIZE##_handler(emu, device->device_data, (page_index * MMU_PG2H_PAGE_SIZE) + offset, value); \
		} else {                                                                                                        \
			uint8_t* pool = (uint8_t*)(pte & MMU_PG2H_PAGE_MASK);                                                   \
			TYPE* host_addr = (TYPE*)&pool[offset];                                                                 \
			*host_addr = htole##SIZE(value);                                                                        \
		}                                                                                                               \
		return ret;                                                                                                     \
	}

// Reading or writing a 8 bit value misaligned shouldn't be possible
static inline uint8_t emu_r8_misaligned(emulator_t* emu, guest_vaddr vaddr) {
	(void)emu;
	(void)vaddr;
	abort();
}

static inline bool emu_w8_misaligned(emulator_t* emu, guest_vaddr vaddr, uint8_t value) {
	(void)emu;
	(void)vaddr;
	(void)value;
	abort();
}

EMU_RX_MISALIGNED(16, uint16_t)
EMU_RX_MISALIGNED(32, uint32_t)
EMU_RX_MISALIGNED(64, uint64_t)
EMU_WX_MISALIGNED(16, uint16_t)
EMU_WX_MISALIGNED(32, uint32_t)
EMU_WX_MISALIGNED(64, uint64_t)

EMU_RX(8, uint8_t)
EMU_RX(16, uint16_t)
EMU_RX(32, uint32_t)
EMU_RX(64, uint64_t)
EMU_WX(8, uint8_t)
EMU_WX(16, uint16_t)
EMU_WX(32, uint32_t)
EMU_WX(64, uint64_t)

uint32_t emu_r32_ins(emulator_t* emu, guest_vaddr vaddr, uint8_t* exception_code, guest_reg* exception_tval) {
	size_t offset = vaddr & MMU_PG2H_OFFSET_MASK;
	// Instruction alignement should be guaranteed by the caller
	assert((offset & 3) == 0);

	*exception_code = (uint8_t)-1;

	guest_paddr paddr;
	if (mmu_vg2pg_should_translate(emu, false)) {
		if (!mmu_vg2pg_translate(emu, MMU_VG2PG_ACCESS_EXEC, vaddr, &paddr)) {
			*exception_code = EXC_INS_PAGE_FAULT;
			*exception_tval = vaddr;
			return 0;
		}
	} else {
		paddr = vaddr;
	}

	mmu_pg2h_pte pte;
	if (!mmu_pg2h_get_pte(emu, paddr, &pte)) {
		*exception_code = EXC_INS_ACCESS_FAULT;
		*exception_tval = paddr;
		return 0;
	}

	if (pte & MMU_PG2H_PTE_TYPE_MMIO) {
		size_t device_index = (pte >> MMU_PG2H_PTE_DEVICE_SHIFT) & MMU_PG2H_PTE_DEVICE_MASK;
		size_t page_index = (pte >> MMU_PG2H_PTE_DEVICE_PAGE_SHIFT) & MMU_PG2H_PTE_DEVICE_PAGE_MASK;
		device_mmio_t* device = &emu->mmio_devices[device_index];
		return device->r32_handler(emu, device->device_data, (page_index * MMU_PG2H_PAGE_SIZE) + offset);
	} else {
		uint8_t* pool = (uint8_t*)(pte & MMU_PG2H_PAGE_MASK);
		uint32_t* value = (uint32_t*)&pool[offset];
		return le32toh(*value);
	}
}

void emu_ebreak(emulator_t* emu) {
	if (emu->cpu.priv_mode != UO_MODE) {
		cpu_throw_exception(emu, EXC_BREAKPOINT, 0);
		return;
	}

	fprintf(stderr, "EBREAK PC=%016" PRIx64 "\n", emu->cpu.pc);
	for (size_t i = 0; i < REG_COUNT; i++) {
		fprintf(stderr, "x%2zu=%016" PRIx64 " ", i, emu->cpu.regs[i]);
		if ((i % 4) == 3) {
			fprintf(stderr, "\n");
		}
	}
}

void emu_ecall(emulator_t* emu) {
	if (emu->cpu.priv_mode != UO_MODE) {
		privilege_mode_t priv_mode = emu->cpu.priv_mode;
		cpu_throw_exception(emu,
				    priv_mode == M_MODE
					    ? EXC_ECALL_FROM_M
				    : priv_mode == S_MODE
					    ? EXC_ECALL_FROM_S
					    : EXC_ECALL_FROM_U,
				    0);
		return;
	}

	// Handle custom emulator calls
	switch (emu->cpu.regs[10]) {
		case 0x50555443:  // "PUTC"
			fputc(emu->cpu.regs[11] & 0xff, stdout);
			fflush(stdout);
			break;
		case 0x45584954:  // "EXIT"
			fprintf(stderr, "guest exited\n");
			exit(emu->cpu.regs[11]);
			break;
		case 0x4754494b: {  // "GTIK"
			struct timespec tp;
			clock_gettime(CLOCK_MONOTONIC, &tp);
			emu->cpu.regs[10] = (tp.tv_sec * 1000) + (tp.tv_nsec / 1000000);
			break;
		}
		case 0x534c4550:  // "SLEP"
			usleep(emu->cpu.regs[11] * 1000);
			break;
		case 0x50494e47:                         // "PING"
			emu->cpu.regs[10] = 0x504f4e47;  // "PONG"
			break;
#ifdef RISCV_EMULATOR_SDL_SUPPORT
		case 0x494e4954:  // "INIT"
			emu_sdl_init(emu, emu->cpu.regs[11], emu->cpu.regs[12]);
			break;
		case 0x44524157:  // "DRAW"
			emu_sdl_draw(emu, emu->cpu.regs[11]);
			break;
		case 0x474b4559: {  // "GKEY"
			unsigned int pressed;
			uint8_t key;
			emu->cpu.regs[10] = emu_sdl_poll_events(emu, &pressed, &key);
			emu->cpu.regs[11] = pressed;
			emu->cpu.regs[12] = key;
			break;
		}
#else
		case 0x494e4954:  // "INIT"
		case 0x44524157:  // "DRAW"
		case 0x474b4559:  // "GKEY"
			fprintf(stderr, "SDL ecalls require RISCV_EMULATOR_SDL_SUPPORT\n");
			emu_ebreak(emu);
			break;
#endif
		default:
			emu_ebreak(emu);
			break;
	}
}

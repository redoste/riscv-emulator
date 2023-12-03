#include <assert.h>
#include <endian.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

#include "cpu.h"
#include "emulator.h"
#include "emulator_sdl.h"
#include "mmu_paging_guest_to_host.h"

void emu_create(emulator_t* emu, guest_reg pc, size_t instruction_cache_bits) {
	emu->pg2h_paging_table = 0;

	memset(&emu->cpu, 0, sizeof(emu->cpu));
	emu->cpu.pc = pc;

	if (instruction_cache_bits > 24) {
		fprintf(stderr, "The number of significant bits for the instruction cache is over 24 bits\n");
		abort();
	}
	const guest_paddr instruction_cache_mask = (1ull << instruction_cache_bits) - 1;
	const size_t instruction_cache_size = (1ull << instruction_cache_bits) * sizeof(emu->cpu.instruction_cache[0]);
	emu->cpu.instruction_cache = malloc(instruction_cache_size);
	emu->cpu.instruction_cache_mask = instruction_cache_mask;
	assert(emu->cpu.instruction_cache != NULL);
	memset(emu->cpu.instruction_cache, 0, instruction_cache_size);

#ifdef RISCV_EMULATOR_SDL_SUPPORT
	memset(&emu->sdl_data, 0, sizeof(emu->sdl_data));
#endif
}

void emu_destroy(emulator_t* emu) {
	mmu_pg2h_free(emu);
	free(emu->cpu.instruction_cache);
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

#define le8toh(x) (x)
#define htole8(x) (x)

#define EMU_RX_MISALIGNED(SIZE, TYPE)                                                    \
	static inline TYPE emu_r##SIZE##_misaligned(emulator_t* emu, guest_paddr addr) { \
		TYPE value = 0;                                                          \
		for (size_t i = 0; i < sizeof(TYPE); i++) {                              \
			value |= (TYPE)emu_r8(emu, addr + i) << (i * 8);                 \
		}                                                                        \
		return value;                                                            \
	}

#define EMU_RX(SIZE, TYPE)                                                                  \
	TYPE emu_r##SIZE(emulator_t* emu, guest_paddr addr) {                               \
		mmu_pg2h_pte pte;                                                           \
		size_t offset = addr & MMU_PG2H_OFFSET_MASK;                                \
		if ((offset & (sizeof(TYPE) - 1)) != 0) {                                   \
			return emu_r##SIZE##_misaligned(emu, addr);                         \
		}                                                                           \
		/* Read across page boudaries should be handled by the misaligned case */   \
		assert(offset + sizeof(TYPE) <= MMU_PG2H_PAGE_SIZE);                        \
		if (mmu_pg2h_get_pte(emu, addr, &pte) &&                                    \
		    (pte & MMU_PG2H_PTE_VALID) &&                                           \
		    !(pte & MMU_PG2H_PTE_TYPE_MMIO)) {                                      \
			uint8_t* pool = (uint8_t*)(pte & MMU_PG2H_PAGE_MASK);               \
			TYPE* value = (TYPE*)&pool[offset];                                 \
			return le##SIZE##toh(*value);                                       \
		}                                                                           \
                                                                                            \
		fprintf(stderr, "invalid r" #SIZE " at %016" PRIx64 " PC=%016" PRIx64 "\n", \
			addr, emu->cpu.pc);                                                 \
		abort();                                                                    \
	}

#define EMU_WX_MISALIGNED(SIZE, TYPE)                                                                \
	static inline void emu_w##SIZE##_misaligned(emulator_t* emu, guest_paddr addr, TYPE value) { \
		for (size_t i = 0; i < sizeof(TYPE); i++) {                                          \
			emu_w8(emu, addr + i, value & 0xff);                                         \
			value >>= 8;                                                                 \
		}                                                                                    \
	}

#define EMU_WX(SIZE, TYPE)                                                                  \
	void emu_w##SIZE(emulator_t* emu, guest_paddr addr, TYPE value) {                   \
		cpu_invalidate_instruction_cache(emu, addr);                                \
		mmu_pg2h_pte pte;                                                           \
		size_t offset = addr & MMU_PG2H_OFFSET_MASK;                                \
		if ((offset & (sizeof(TYPE) - 1)) != 0) {                                   \
			emu_w##SIZE##_misaligned(emu, addr, value);                         \
			return;                                                             \
		}                                                                           \
		/* Write across page boudaries should be handled by the misaligned case */  \
		assert(offset + sizeof(TYPE) <= MMU_PG2H_PAGE_SIZE);                        \
		if (mmu_pg2h_get_pte(emu, addr, &pte) &&                                    \
		    (pte & MMU_PG2H_PTE_VALID) &&                                           \
		    !(pte & MMU_PG2H_PTE_TYPE_MMIO)) {                                      \
			uint8_t* pool = (uint8_t*)(pte & MMU_PG2H_PAGE_MASK);               \
			TYPE* host_addr = (TYPE*)&pool[offset];                             \
			*host_addr = htole##SIZE(value);                                    \
			return;                                                             \
		}                                                                           \
                                                                                            \
		fprintf(stderr, "invalid w" #SIZE " at %016" PRIx64 " PC=%016" PRIx64 "\n", \
			addr, emu->cpu.pc);                                                 \
		abort();                                                                    \
	}

// Reading or writing a 8 bit value misaligned shouldn't be possible
static inline uint8_t emu_r8_misaligned(emulator_t* emu, guest_paddr addr) {
	(void)emu;
	(void)addr;
	abort();
}

static inline void emu_w8_misaligned(emulator_t* emu, guest_paddr addr, uint8_t value) {
	(void)emu;
	(void)addr;
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

void emu_ebreak(emulator_t* emu) {
	fprintf(stderr, "EBREAK PC=%016" PRIx64 "\n", emu->cpu.pc);
	for (size_t i = 0; i < REG_COUNT; i++) {
		fprintf(stderr, "x%2zu=%016" PRIx64 " ", i, emu->cpu.regs[i]);
		if ((i % 4) == 3) {
			fprintf(stderr, "\n");
		}
	}
}

void emu_ecall(emulator_t* emu) {
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

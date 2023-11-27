#include <assert.h>
#include <endian.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cpu.h"
#include "emulator.h"
#include "emulator_sdl.h"

void emu_create(emulator_t* emu, guest_paddr rom_base, size_t rom_size, guest_paddr ram_base, size_t ram_size) {
	uint8_t* rom_pool = malloc(rom_size);
	uint8_t* ram_pool = malloc(ram_size);
	assert(rom_pool != NULL && ram_pool != NULL);

	if ((ram_base >= rom_base && ram_base < rom_base + rom_size) ||
	    (rom_base >= ram_base && rom_base < ram_base + ram_size)) {
		fprintf(stderr, "ROM and RAM overlap\n");
		abort();
	}

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

#ifdef RISCV_EMULATOR_SDL_SUPPORT
	memset(&emu->sdl_data, 0, sizeof(emu->sdl_data));
#endif
}

void emu_destroy(emulator_t* emu) {
	free(emu->rom.pool);
	free(emu->ram.pool);
}

#define le8toh(x) (x)
#define htole8(x) (x)

#define EMU_RX(SIZE, TYPE)                                                                  \
	TYPE emu_r##SIZE(emulator_t* emu, guest_paddr addr) {                               \
		if (addr - emu->rom.base < emu->rom.size &&                                 \
		    addr - emu->rom.base + sizeof(TYPE) <= emu->rom.size) {                 \
			TYPE* value = (TYPE*)&emu->rom.pool[addr - emu->rom.base];          \
			return le##SIZE##toh(*value);                                       \
		} else if (addr - emu->ram.base < emu->ram.size &&                          \
			   addr - emu->ram.base + sizeof(TYPE) <= emu->ram.size) {          \
			TYPE* value = (TYPE*)&emu->ram.pool[addr - emu->ram.base];          \
			return le##SIZE##toh(*value);                                       \
		}                                                                           \
                                                                                            \
		fprintf(stderr, "invalid r" #SIZE " at %016" PRIx64 " PC=%016" PRIx64 "\n", \
			addr, emu->cpu.pc);                                                 \
		abort();                                                                    \
	}

#define EMU_WX(SIZE, TYPE)                                                                  \
	void emu_w##SIZE(emulator_t* emu, guest_paddr addr, TYPE value) {                   \
		if (addr - emu->rom.base < emu->rom.size &&                                 \
		    addr - emu->rom.base + sizeof(TYPE) <= emu->rom.size) {                 \
			TYPE* host_addr = (TYPE*)&emu->rom.pool[addr - emu->rom.base];      \
			*host_addr = htole##SIZE(value);                                    \
			return;                                                             \
		} else if (addr - emu->ram.base < emu->ram.size &&                          \
			   addr - emu->ram.base + sizeof(TYPE) <= emu->ram.size) {          \
			TYPE* host_addr = (TYPE*)&emu->ram.pool[addr - emu->ram.base];      \
			*host_addr = htole##SIZE(value);                                    \
			return;                                                             \
		}                                                                           \
                                                                                            \
		fprintf(stderr, "invalid w" #SIZE " at %016" PRIx64 " PC=%016" PRIx64 "\n", \
			addr, emu->cpu.pc);                                                 \
		abort();                                                                    \
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
		case 0x44524157: {  // "DRAW"
			guest_paddr addr = emu->cpu.regs[11];
			uint8_t* frame;
			size_t max_frame_size;
			if (addr - emu->rom.base < emu->rom.size) {
				frame = &emu->rom.pool[addr - emu->rom.base];
				max_frame_size = emu->rom.size - (addr - emu->rom.base);
			} else if (addr - emu->ram.base < emu->ram.size) {
				frame = &emu->ram.pool[addr - emu->ram.base];
				max_frame_size = emu->ram.size - (addr - emu->ram.base);
			} else {
				fprintf(stderr, "invalid DRAW at %016" PRIx64 " PC=%016" PRIx64 "\n",
					addr, emu->cpu.pc);
				abort();
			}
			emu_sdl_draw(emu, frame, max_frame_size);
			break;
		}
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

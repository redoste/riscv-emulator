#ifdef RISCV_EMULATOR_SDL_SUPPORT

#include <stdbool.h>
#include <stdint.h>

#include "device_framebuffer.h"
#include "emulator.h"
#include "emulator_sdl.h"

static void framebuffer_free(emulator_t* emu, void* device_data) {
	(void)emu;

	free(device_data);
}

static void framebuffer_update(emulator_t* emu, void* device_data) {
	guest_paddr* framebuffer_base = (guest_paddr*)device_data;
	emu_sdl_draw(emu, *framebuffer_base);
}

bool framebuffer_create(emulator_t* emu, guest_paddr base, int width, int height) {
	guest_paddr* framebuffer_base = malloc(sizeof(guest_paddr));
	assert(framebuffer_base != NULL);
	*framebuffer_base = base;

	if (!(emu->sdl_data.window && emu->sdl_data.renderer && emu->sdl_data.texture)) {
		emu_sdl_init(emu, width, height);
	}

	size_t framebuffer_size = width * height * 4;
	if (framebuffer_size % MMU_PG2H_PAGE_SIZE != 0) {
		framebuffer_size += MMU_PG2H_PAGE_SIZE - (framebuffer_size % MMU_PG2H_PAGE_SIZE);
	}

	device_mmio_t device = {
		.device_data = framebuffer_base,
		.free_handler = framebuffer_free,
		.update_handler = framebuffer_update,
	};

	if (!emu_map_memory(emu, base, framebuffer_size) ||
	    !emu_add_mmio_device(emu, base, 0, &device)) {
		free(framebuffer_base);
		return false;
	}

	return true;
}

#endif

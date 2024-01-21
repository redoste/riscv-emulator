#include <stdbool.h>
#include <unistd.h>

#include "device_clint.h"
#include "device_framebuffer.h"
#include "device_plic.h"
#include "device_syscon.h"
#include "device_uart8250.h"
#include "device_virtio_block.h"
#include "device_virtio_input.h"
#include "devices.h"
#include "emulator.h"
#include "isa.h"

// The created machine is following the device tree described in `linux/emulator.dts`

bool devices_create_virt_machine(emulator_t* emu, const char* hdd_file) {
	FILE* hdd_image = fopen(hdd_file, "r+b");
	if (hdd_image == NULL) {
		perror("fopen");
		return false;
	}

#define CREATE_CHECKED(name, ...)                                                         \
	do {                                                                              \
		if (!name##_create(emu, __VA_ARGS__)) {                                   \
			fprintf(stderr, "Unable to create the deivce : \"" #name "\"\n"); \
			return false;                                                     \
		}                                                                         \
	} while (0)

	CREATE_CHECKED(uart8250, 0x10000000, 1, STDOUT_FILENO, STDIN_FILENO);
	CREATE_CHECKED(clint, 0x20000000);
	CREATE_CHECKED(plic, 0x30000000);
	CREATE_CHECKED(virtio_block, 0x40000000, 2, hdd_image);
#ifdef RISCV_EMULATOR_SDL_SUPPORT
	CREATE_CHECKED(virtio_input, 0x40001000, 3);
	CREATE_CHECKED(framebuffer, 0x50000000, 800, 600);
#endif
	CREATE_CHECKED(syscon, 0x60000000);

#undef CREATE_CHECKED

	return true;
}

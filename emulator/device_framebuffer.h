#ifndef DEVICE_FRAMEBUFFER_H
#define DEVICE_FRAMEBUFFER_H

#ifdef RISCV_EMULATOR_SDL_SUPPORT

#include <stdbool.h>

#include "emulator.h"
#include "isa.h"

/* framebuffer_create : create and attach a framebuffer to the emulator
 *                      returns true if the framebuffer was successfully created
 *     emulator_t* emu  : pointer to the emulator
 *     guest_paddr base : base address of the framebuffer
 *     int width        : width of the framebuffer
 *     int height       : height of the framebuffer
 */
bool framebuffer_create(emulator_t* emu, guest_paddr base, int width, int height);

#endif

#endif

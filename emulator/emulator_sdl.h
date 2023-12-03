#ifndef EMULATOR_SDL_H
#define EMULATOR_SDL_H

#ifdef RISCV_EMULATOR_SDL_SUPPORT

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "isa.h"

// NOTE : forward declaration to deal with a cyclic dependency with emulator.h
typedef struct emulator_t emulator_t;

/* emu_sdl_data_t : structure storing the current state of the SDL subsystem
 */
typedef struct emu_sdl_data_t {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	unsigned int width;
	unsigned int height;
	struct timespec previous_frame_time;
} emu_sdl_data_t;

/* emu_sdl_init : initialize the SDL subsystem
 *     emulator_t* emu : pointer to the emulator
 *     int width       : width of the window to create
 *     int height      : height of the window to create
 */
void emu_sdl_init(emulator_t* emu, int width, int height);

/* emu_sdl_draw : draw a frame to the SDL window
 *     emulator_t* emu       : pointer to the emulator
 *     guest_paddr addr      : address of the frame to draw in the guest memory
 */
void emu_sdl_draw(emulator_t* emu, guest_paddr addr);

/* emu_sdl_poll_events : poll events from the SDL window
 *                       returns 1 if a keyboard event was caught
 *                       returns 0 otherwise
 *     emulator_t* emu       : pointer to the emulator
 *     unsigned int* pressed : pointer to the value set to 1 in case the keyboard event is about a key being pressed
 *                                                  set to 0 in case the keyboard event is about a key being released
 *     uint8_t* key          : pointer to the value set to the correct keycode in case of a keyboard event
 *                             the keycodes are following doomgeneric's map
 */
unsigned int emu_sdl_poll_events(emulator_t* emu, unsigned int* pressed, uint8_t* key);

#endif

#endif

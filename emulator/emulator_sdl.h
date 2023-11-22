#ifndef EMULATOR_SDL_H
#define EMULATOR_SDL_H

#include <stddef.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>

typedef struct emulator_t emulator_t;

typedef struct emu_sdl_data_t {
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* texture;
	unsigned int width;
	unsigned int height;
	struct timespec previous_frame_time;
} emu_sdl_data_t;

void emu_sdl_init(emulator_t* emu, int width, int height);
void emu_sdl_draw(emulator_t* emu, uint8_t* frame, size_t max_frame_size);
unsigned int emu_sdl_poll_events(emulator_t* emu, unsigned int* pressed, uint8_t* key);

#endif

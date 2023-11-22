#include <assert.h>
#include <stdint.h>
#include <time.h>

#include <SDL2/SDL.h>

#include "emulator.h"
#include "emulator_sdl.h"

void emu_sdl_init(emulator_t* emu, int width, int height) {
	if (emu->sdl_data.window || emu->sdl_data.renderer || emu->sdl_data.texture) {
		fprintf(stderr, "guest called INIT emucall multiple times\n");
		abort();
	}

	/* NOTE : By default SDL will catch SIGINT, we want to keep the default
	 *        esignal handler to be able to asily kill the emulator in case
	 *        the guest code crashed
	 */
	SDL_SetHint(SDL_HINT_NO_SIGNAL_HANDLERS, "1");
	int ret = SDL_Init(SDL_INIT_VIDEO);
	assert(ret == 0);

	SDL_Window* window = SDL_CreateWindow("riscv-emulator", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
					      width, height, SDL_WINDOW_SHOWN);
	assert(window != NULL);

	SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
	assert(renderer != NULL);

	ret = SDL_RenderClear(renderer);
	assert(ret == 0);
	SDL_RenderPresent(renderer);

	assert(SDL_PIXELTYPE(SDL_PIXELFORMAT_RGB888) == SDL_PIXELTYPE_PACKED32);
	assert(SDL_BITSPERPIXEL(SDL_PIXELFORMAT_RGB888) == 24);
	assert(SDL_PIXELORDER(SDL_PIXELFORMAT_RGB888) == SDL_PACKEDORDER_XRGB);

	SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET,
						 width, height);
	assert(texture != NULL);

	emu->sdl_data.window = window;
	emu->sdl_data.renderer = renderer;
	emu->sdl_data.texture = texture;

	assert(width > 0 && height > 0);
	emu->sdl_data.width = width;
	emu->sdl_data.height = height;
}

void emu_sdl_draw(emulator_t* emu, uint8_t* frame, size_t max_frame_size) {
	if (!(emu->sdl_data.window && emu->sdl_data.renderer && emu->sdl_data.texture)) {
		fprintf(stderr, "guest called DRAW emucall before INIT emucall\n");
		abort();
	}

	size_t frame_size = emu->sdl_data.width * emu->sdl_data.height * sizeof(uint32_t);
	if (frame_size > max_frame_size) {
		fprintf(stderr, "guest called DRAW with a frame near the end of a memory pool\n");
		abort();
	}

	int ret = SDL_UpdateTexture(emu->sdl_data.texture, NULL, frame, emu->sdl_data.width * sizeof(uint32_t));
	assert(ret == 0);
	ret = SDL_RenderClear(emu->sdl_data.renderer);
	assert(ret == 0);
	ret = SDL_RenderCopy(emu->sdl_data.renderer, emu->sdl_data.texture, NULL, NULL);
	assert(ret == 0);
	SDL_RenderPresent(emu->sdl_data.renderer);

	struct timespec current_time;
	clock_gettime(CLOCK_MONOTONIC, &current_time);
	unsigned long long frame_time = (current_time.tv_sec - emu->sdl_data.previous_frame_time.tv_sec) * 1000 +
					(current_time.tv_nsec - emu->sdl_data.previous_frame_time.tv_nsec) / 1000000;
	emu->sdl_data.previous_frame_time = current_time;

	if (frame_time > 0) {
		char title_buffer[64] = {0};
		snprintf(title_buffer, sizeof(title_buffer) - 1, "riscv-emulator (FPS:%3llu)", 1000 / frame_time);
		SDL_SetWindowTitle(emu->sdl_data.window, title_buffer);
	} else {
		SDL_SetWindowTitle(emu->sdl_data.window, "riscv-emulator (FPS: inf)");
	}
}

unsigned int emu_sdl_poll_events(emulator_t* emu, unsigned int* pressed, uint8_t* key) {
	if (!(emu->sdl_data.window && emu->sdl_data.renderer && emu->sdl_data.texture)) {
		fprintf(stderr, "guest called GKEY emucall before INIT emucall\n");
		abort();
	}

	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		if (event.type == SDL_QUIT) {
			fprintf(stderr, "SDL_QUIT forced an exit\n");
			exit(0);
		} else if ((event.type == SDL_KEYDOWN || event.type == SDL_KEYUP) &&
			   event.key.repeat == 0) {
			switch (event.key.keysym.sym) {
				case SDLK_UP:
					*key = 0xad;  // KEY_UPARROW
					break;
				case SDLK_DOWN:
					*key = 0xaf;  // KEY_DOWNARROW
					break;
				case SDLK_LEFT:
					*key = 0xac;  // KEY_LEFTARROW
					break;
				case SDLK_RIGHT:
					*key = 0xae;  // KEY_RIGHTARROW
					break;
				case SDLK_LCTRL:
				case SDLK_RCTRL:
					*key = 0xa3;  // KEY_FIRE
					break;
				case SDLK_SPACE:
					*key = 0xa2;  // KEY_USE
					break;
				case SDLK_LALT:
				case SDLK_RALT:
					*key = 0x80 + 0x38;  // KEY_RALT
					break;
				case SDLK_LSHIFT:
				case SDLK_RSHIFT:
					*key = 0x80 + 0x36;  // KEY_RSHIFT
					break;
				default:
					*key = event.key.keysym.sym;
					break;
			}
			*pressed = event.key.state;
			return 1;
		}
	}
	return 0;
}

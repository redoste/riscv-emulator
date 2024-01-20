#ifdef RISCV_EMULATOR_SDL_SUPPORT

#include <stdbool.h>
#include <stdio.h>

#include <SDL2/SDL.h>
#include <linux/input-event-codes.h>

#include "device_virtio.h"
#include "emulator.h"
#include "isa.h"

#define VIRTIO_INPUT_DEVICE_ID 18

#define VIRTIO_INPUT_CFG_UNSET     0x00
#define VIRTIO_INPUT_CFG_ID_NAME   0x01
#define VIRTIO_INPUT_CFG_ID_SERIAL 0x02
#define VIRTIO_INPUT_CFG_ID_DEVIDS 0x03
#define VIRTIO_INPUT_CFG_PROP_BITS 0x10
#define VIRTIO_INPUT_CFG_EV_BITS   0x11
#define VIRTIO_INPUT_CFG_ABS_INFO  0x12

#define VIRTIO_INPUT_CFG_SELECT_BASE 0
#define VIRTIO_INPUT_CFG_SUBSEL_BASE 1
#define VIRTIO_INPUT_CFG_SIZE_BASE   2
#define VIRTIO_INPUT_CFG_DATA_BASE   8

#define VIRTIO_INPUT_EVENTQ   0
#define VIRTIO_INPUT_STATUSQ  1
#define VIRTIO_INPUT_N_QUEUES 2

#define VIRTIO_INPUT_QUEUE_NUM_MAX 128

#define VIRTIO_INPUT_RING_BUF_CAP 128

typedef struct virtio_input_event_t {
	uint16_t type;
	uint16_t code;
	uint32_t value;
} virtio_input_event_t;

typedef struct virtio_input_t {
	uint8_t config_select;
	uint8_t config_subsel;

	struct {
		size_t read_end;
		size_t write_end;
		virtio_input_event_t buf[VIRTIO_INPUT_RING_BUF_CAP];
	} ring;
} virtio_input_t;

static void virtio_input_free(emulator_t* emu, virtio_t* virtio) {
	(void)emu;

	virtio_input_t* virtio_input = (virtio_input_t*)virtio->device_data;
	free(virtio_input);
}

static const uint8_t VIRTIO_SDL_SCANCODE_EVDEV_CODE_TABLE[] = {
	[SDL_SCANCODE_A] = KEY_A,
	[SDL_SCANCODE_B] = KEY_B,
	[SDL_SCANCODE_C] = KEY_C,
	[SDL_SCANCODE_D] = KEY_D,
	[SDL_SCANCODE_E] = KEY_E,
	[SDL_SCANCODE_F] = KEY_F,
	[SDL_SCANCODE_G] = KEY_G,
	[SDL_SCANCODE_H] = KEY_H,
	[SDL_SCANCODE_I] = KEY_I,
	[SDL_SCANCODE_J] = KEY_J,
	[SDL_SCANCODE_K] = KEY_K,
	[SDL_SCANCODE_L] = KEY_L,
	[SDL_SCANCODE_M] = KEY_M,
	[SDL_SCANCODE_N] = KEY_N,
	[SDL_SCANCODE_O] = KEY_O,
	[SDL_SCANCODE_P] = KEY_P,
	[SDL_SCANCODE_Q] = KEY_Q,
	[SDL_SCANCODE_R] = KEY_R,
	[SDL_SCANCODE_S] = KEY_S,
	[SDL_SCANCODE_T] = KEY_T,
	[SDL_SCANCODE_U] = KEY_U,
	[SDL_SCANCODE_V] = KEY_V,
	[SDL_SCANCODE_W] = KEY_W,
	[SDL_SCANCODE_X] = KEY_X,
	[SDL_SCANCODE_Y] = KEY_Y,
	[SDL_SCANCODE_Z] = KEY_Z,
	[SDL_SCANCODE_1] = KEY_1,
	[SDL_SCANCODE_2] = KEY_2,
	[SDL_SCANCODE_3] = KEY_3,
	[SDL_SCANCODE_4] = KEY_4,
	[SDL_SCANCODE_5] = KEY_5,
	[SDL_SCANCODE_6] = KEY_6,
	[SDL_SCANCODE_7] = KEY_7,
	[SDL_SCANCODE_8] = KEY_8,
	[SDL_SCANCODE_9] = KEY_9,
	[SDL_SCANCODE_0] = KEY_0,
	[SDL_SCANCODE_RETURN] = KEY_ENTER,
	[SDL_SCANCODE_ESCAPE] = KEY_ESC,
	[SDL_SCANCODE_BACKSPACE] = KEY_BACKSPACE,
	[SDL_SCANCODE_TAB] = KEY_TAB,
	[SDL_SCANCODE_SPACE] = KEY_SPACE,
	[SDL_SCANCODE_MINUS] = KEY_MINUS,
	[SDL_SCANCODE_EQUALS] = KEY_EQUAL,
	[SDL_SCANCODE_LEFTBRACKET] = KEY_LEFTBRACE,
	[SDL_SCANCODE_RIGHTBRACKET] = KEY_RIGHTBRACE,
	[SDL_SCANCODE_BACKSLASH] = KEY_BACKSLASH,
	[SDL_SCANCODE_SEMICOLON] = KEY_SEMICOLON,
	[SDL_SCANCODE_APOSTROPHE] = KEY_APOSTROPHE,
	[SDL_SCANCODE_GRAVE] = KEY_GRAVE,
	[SDL_SCANCODE_COMMA] = KEY_COMMA,
	[SDL_SCANCODE_PERIOD] = KEY_DOT,
	[SDL_SCANCODE_SLASH] = KEY_SLASH,
	[SDL_SCANCODE_CAPSLOCK] = KEY_CAPSLOCK,
	[SDL_SCANCODE_F1] = KEY_F1,
	[SDL_SCANCODE_F2] = KEY_F2,
	[SDL_SCANCODE_F3] = KEY_F3,
	[SDL_SCANCODE_F4] = KEY_F4,
	[SDL_SCANCODE_F5] = KEY_F5,
	[SDL_SCANCODE_F6] = KEY_F6,
	[SDL_SCANCODE_F7] = KEY_F7,
	[SDL_SCANCODE_F8] = KEY_F8,
	[SDL_SCANCODE_F9] = KEY_F9,
	[SDL_SCANCODE_F10] = KEY_F10,
	[SDL_SCANCODE_F11] = KEY_F11,
	[SDL_SCANCODE_F12] = KEY_F12,
	[SDL_SCANCODE_PRINTSCREEN] = KEY_SYSRQ,
	[SDL_SCANCODE_SCROLLLOCK] = KEY_SCROLLLOCK,
	[SDL_SCANCODE_PAUSE] = KEY_PAUSE,
	[SDL_SCANCODE_INSERT] = KEY_INSERT,
	[SDL_SCANCODE_HOME] = KEY_HOME,
	[SDL_SCANCODE_PAGEUP] = KEY_PAGEUP,
	[SDL_SCANCODE_DELETE] = KEY_DELETE,
	[SDL_SCANCODE_END] = KEY_END,
	[SDL_SCANCODE_PAGEDOWN] = KEY_PAGEDOWN,
	[SDL_SCANCODE_RIGHT] = KEY_RIGHT,
	[SDL_SCANCODE_LEFT] = KEY_LEFT,
	[SDL_SCANCODE_DOWN] = KEY_DOWN,
	[SDL_SCANCODE_UP] = KEY_UP,
	[SDL_SCANCODE_KP_DIVIDE] = KEY_KPSLASH,
	[SDL_SCANCODE_KP_MULTIPLY] = KEY_KPASTERISK,
	[SDL_SCANCODE_KP_MINUS] = KEY_KPMINUS,
	[SDL_SCANCODE_KP_PLUS] = KEY_KPPLUS,
	[SDL_SCANCODE_KP_ENTER] = KEY_KPENTER,
	[SDL_SCANCODE_KP_1] = KEY_KP1,
	[SDL_SCANCODE_KP_2] = KEY_KP2,
	[SDL_SCANCODE_KP_3] = KEY_KP3,
	[SDL_SCANCODE_KP_4] = KEY_KP4,
	[SDL_SCANCODE_KP_5] = KEY_KP5,
	[SDL_SCANCODE_KP_6] = KEY_KP6,
	[SDL_SCANCODE_KP_7] = KEY_KP7,
	[SDL_SCANCODE_KP_8] = KEY_KP8,
	[SDL_SCANCODE_KP_9] = KEY_KP9,
	[SDL_SCANCODE_KP_0] = KEY_KP0,
	[SDL_SCANCODE_KP_PERIOD] = KEY_KPDOT,
	[SDL_SCANCODE_NONUSBACKSLASH] = KEY_102ND,
	[SDL_SCANCODE_LCTRL] = KEY_LEFTCTRL,
	[SDL_SCANCODE_LSHIFT] = KEY_LEFTSHIFT,
	[SDL_SCANCODE_LALT] = KEY_LEFTALT,
	[SDL_SCANCODE_LGUI] = KEY_LEFTMETA,
	[SDL_SCANCODE_RCTRL] = KEY_RIGHTCTRL,
	[SDL_SCANCODE_RSHIFT] = KEY_RIGHTSHIFT,
	[SDL_SCANCODE_RALT] = KEY_RIGHTALT,
	[SDL_SCANCODE_RGUI] = KEY_RIGHTMETA,
	[SDL_SCANCODE_MENU] = KEY_COMPOSE,
};

static bool virtio_input_req(emulator_t* emu, virtio_t* virtio, size_t queue_index, virtio_desc_t* desc) {
	virtio_input_t* virtio_input = (virtio_input_t*)virtio->device_data;

	if (queue_index == VIRTIO_INPUT_EVENTQ) {
		if (virtio_input->ring.read_end == virtio_input->ring.write_end) {
			// We don't have any events pending in the ring buffer
			return false;
		}

		if (desc->bufs_len < 1 || desc->bufs[0].len < 8 || !(desc->bufs[0].flags & VIRTQ_DESC_F_WRITE)) {
			return false;
		}

		/* struct virtio_input_event {
		 *   le16 type;
		 *   le16 code;
		 *   le32 value;
		 * };
		 */
		bool ret = true;
		virtio_input_event_t* event = &virtio_input->ring.buf[virtio_input->ring.read_end];
		ret &= emu_physical_w16(emu, desc->bufs[0].addr + 0, event->type);
		ret &= emu_physical_w16(emu, desc->bufs[0].addr + 2, event->code);
		ret &= emu_physical_w32(emu, desc->bufs[0].addr + 4, event->value);
		virtio_input->ring.read_end = (virtio_input->ring.read_end + 1) % VIRTIO_INPUT_RING_BUF_CAP;

		return ret;
	} else {
		assert(queue_index == VIRTIO_INPUT_STATUSQ);
		// This is where we would handle keyboard LEDs if we cared
		return true;
	}
}

static void virtio_input_update(emulator_t* emu, virtio_t* virtio) {
	virtio_input_t* virtio_input = (virtio_input_t*)virtio->device_data;

	if (!(emu->sdl_data.window && emu->sdl_data.renderer && emu->sdl_data.texture)) {
		// We expect an other device (such as a framebuffer) to have initialized SDL
		return;
	}

#define VIRTIO_INPUT_APPEND_TO_RING(ev_type, ev_code, ev_value)                                                \
	do {                                                                                                   \
		virtio_input_event_t* evdev_event = &virtio_input->ring.buf[virtio_input->ring.write_end];     \
		evdev_event->type = (ev_type);                                                                 \
		evdev_event->code = (ev_code);                                                                 \
		evdev_event->value = (ev_value);                                                               \
		virtio_input->ring.write_end = (virtio_input->ring.write_end + 1) % VIRTIO_INPUT_RING_BUF_CAP; \
		do_syn |= true;                                                                                \
	} while (0)

	bool do_syn = false;
	SDL_Event event;
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
			case SDL_QUIT:
				fprintf(stderr, "SDL_QUIT forced an exit\n");
				exit(0);
				break;
			case SDL_KEYDOWN:
			case SDL_KEYUP: {
				SDL_Scancode sdl_scancode = event.key.keysym.scancode;
				uint32_t evdev_value = event.key.repeat ? 2 : event.key.state;
				if (sdl_scancode < sizeof(VIRTIO_SDL_SCANCODE_EVDEV_CODE_TABLE) / sizeof(VIRTIO_SDL_SCANCODE_EVDEV_CODE_TABLE[0]) &&
				    VIRTIO_SDL_SCANCODE_EVDEV_CODE_TABLE[sdl_scancode] != KEY_RESERVED) {
					uint16_t evdev_code = VIRTIO_SDL_SCANCODE_EVDEV_CODE_TABLE[sdl_scancode];
					VIRTIO_INPUT_APPEND_TO_RING(EV_KEY, evdev_code, evdev_value);
				}

				if (sdl_scancode == SDL_SCANCODE_SCROLLLOCK && emu->sdl_data.mouse_grabbed) {
					SDL_SetRelativeMouseMode(SDL_FALSE);
					emu->sdl_data.mouse_grabbed = false;
				}
			} break;
			case SDL_MOUSEMOTION: {
				if (event.motion.xrel != 0) {
					VIRTIO_INPUT_APPEND_TO_RING(EV_REL, REL_X, (int32_t)event.motion.xrel);
				}
				if (event.motion.yrel != 0) {
					VIRTIO_INPUT_APPEND_TO_RING(EV_REL, REL_Y, (int32_t)event.motion.yrel);
				}
			} break;
			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP: {
				uint16_t evdev_code = 0;
				if (event.button.button == SDL_BUTTON_LEFT) {
					evdev_code = BTN_LEFT;
				} else if (event.button.button == SDL_BUTTON_RIGHT) {
					evdev_code = BTN_RIGHT;
				} else if (event.button.button == SDL_BUTTON_MIDDLE) {
					evdev_code = BTN_MIDDLE;
				}

				if (!emu->sdl_data.mouse_grabbed) {
					SDL_SetRelativeMouseMode(SDL_TRUE);
					emu->sdl_data.mouse_grabbed = true;
				}

				if (evdev_code != 0) {
					VIRTIO_INPUT_APPEND_TO_RING(EV_KEY, evdev_code, event.button.state);
				}
			} break;
			case SDL_MOUSEWHEEL: {
				if (event.wheel.y != 0) {
					VIRTIO_INPUT_APPEND_TO_RING(EV_REL, REL_WHEEL, (int32_t)event.wheel.y);
				}
				if (event.wheel.x != 0) {
					VIRTIO_INPUT_APPEND_TO_RING(EV_REL, REL_HWHEEL, (int32_t)event.wheel.x);
				}
			} break;
			default:
				break;
		}
	}

	if (do_syn) {
		VIRTIO_INPUT_APPEND_TO_RING(EV_SYN, SYN_REPORT, 0);
	}

#undef VIRTIO_INPUT_APPEND_TO_RING

	if (virtio->queues[VIRTIO_INPUT_EVENTQ].queue_ready != 0) {
		bool ret = true;
		while (virtio_input->ring.read_end != virtio_input->ring.write_end && ret) {
			virtio_desc_t desc;
			if (!virtio_read_and_use_desc(emu, virtio, VIRTIO_INPUT_EVENTQ, &desc, 8) ||
			    desc.bufs_len < 1 || desc.bufs[0].len < 8 || !(desc.bufs[0].flags & VIRTQ_DESC_F_WRITE)) {
				/* If no descriptors are available, we will be notified by the driver when new ones are available
				 * and the request handler `virtio_input_req` will continue emptying the ring buffer.
				 */
				break;
			}

			/* struct virtio_input_event {
			 *   le16 type;
			 *   le16 code;
			 *   le32 value;
			 * };
			 */
			virtio_input_event_t* event = &virtio_input->ring.buf[virtio_input->ring.read_end];
			ret &= emu_physical_w16(emu, desc.bufs[0].addr + 0, event->type);
			ret &= emu_physical_w16(emu, desc.bufs[0].addr + 2, event->code);
			ret &= emu_physical_w32(emu, desc.bufs[0].addr + 4, event->value);
			virtio_input->ring.read_end = (virtio_input->ring.read_end + 1) % VIRTIO_INPUT_RING_BUF_CAP;
		}
	}
}

static const char* const VIRTIO_INPUT_DEVICE_NAME = "riscv-emulator virtio-input";
static const char* const VIRTIO_INPUT_DEVICE_SERIAL = VIRTIO_INPUT_DEVICE_NAME;

static uint8_t virtio_input_config_size(uint8_t select, uint8_t subsel) {
	switch (select) {
		case VIRTIO_INPUT_CFG_ID_NAME:
			return strlen(VIRTIO_INPUT_DEVICE_NAME);
		case VIRTIO_INPUT_CFG_ID_SERIAL:
			return strlen(VIRTIO_INPUT_DEVICE_SERIAL);
		case VIRTIO_INPUT_CFG_PROP_BITS:
			return 1;
		case VIRTIO_INPUT_CFG_EV_BITS:
			return (subsel == EV_KEY || subsel == EV_REL) ? 128 : 0;
		default:
			return 0;
	}
}

static uint8_t virtio_input_config_data(uint8_t select, uint8_t subsel, size_t offset) {
	switch (select) {
		case VIRTIO_INPUT_CFG_ID_NAME:
			return VIRTIO_INPUT_DEVICE_NAME[offset];
		case VIRTIO_INPUT_CFG_ID_SERIAL:
			return VIRTIO_INPUT_DEVICE_SERIAL[offset];
		case VIRTIO_INPUT_CFG_PROP_BITS:
			return 0;
		case VIRTIO_INPUT_CFG_EV_BITS:
			/* We notify that we support all the EV_KEY and EV_REL events,
			 * it's a bit bloated but it works.
			 */
			return (subsel == EV_KEY || subsel == EV_REL) ? 0xff : 0;
		default:
			return 0;
	}
}

static uint8_t virtio_input_r8_config(emulator_t* emu, virtio_t* virtio, guest_paddr addr) {
	virtio_input_t* virtio_input = (virtio_input_t*)virtio->device_data;
	switch (addr) {
		case VIRTIO_INPUT_CFG_SELECT_BASE:
			return virtio_input->config_select;
		case VIRTIO_INPUT_CFG_SUBSEL_BASE:
			return virtio_input->config_subsel;
		case VIRTIO_INPUT_CFG_SIZE_BASE:
			return virtio_input_config_size(virtio_input->config_select, virtio_input->config_subsel);
		default: {
			uint8_t config_data_size = virtio_input_config_size(virtio_input->config_select, virtio_input->config_subsel);
			if (addr >= VIRTIO_INPUT_CFG_DATA_BASE && addr - VIRTIO_INPUT_CFG_DATA_BASE < config_data_size) {
				return virtio_input_config_data(virtio_input->config_select, virtio_input->config_subsel,
								addr - VIRTIO_INPUT_CFG_DATA_BASE);
			} else {
				cpu_throw_exception(emu, EXC_LOAD_ACCESS_FAULT, virtio->base + VIRTIO_CONFIG_BASE + addr);
				return 0;
			}
		}
	}
}

static void virtio_input_w8_config(emulator_t* emu, virtio_t* virtio, guest_paddr addr, uint8_t value) {
	virtio_input_t* virtio_input = (virtio_input_t*)virtio->device_data;
	switch (addr) {
		case VIRTIO_INPUT_CFG_SELECT_BASE:
			virtio_input->config_select = value;
			break;
		case VIRTIO_INPUT_CFG_SUBSEL_BASE:
			virtio_input->config_subsel = value;
			break;
		default:
			cpu_throw_exception(emu, EXC_STORE_ACCESS_FAULT, virtio->base + VIRTIO_CONFIG_BASE + addr);
			break;
	}
}

static const virtio_handlers_t virtio_input_handlers = {
	.free_handler = virtio_input_free,
	.req_handler = virtio_input_req,
	.update_handler = virtio_input_update,
	.config_r8_handler = virtio_input_r8_config,
	.config_w8_handler = virtio_input_w8_config,
};

bool virtio_input_create(emulator_t* emu, guest_paddr base, size_t int_number) {
	virtio_input_t* virtio_input = malloc(sizeof(virtio_input_t));
	assert(virtio_input != NULL);
	memset(virtio_input, 0, sizeof(virtio_input_t));

	return virtio_create(emu, base, int_number, VIRTIO_INPUT_DEVICE_ID, VIRTIO_INPUT_N_QUEUES,
			     VIRTIO_INPUT_QUEUE_NUM_MAX, &virtio_input_handlers, virtio_input);
}

#endif

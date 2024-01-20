#ifndef DEVICE_VIRTIO_INPUT_H
#define DEVICE_VIRTIO_INPUT_H

#ifdef RISCV_EMULATOR_SDL_SUPPORT

#include <stdbool.h>

#include "emulator.h"
#include "isa.h"

/* virtio_input_create : create and attach a virtio input device to the emulator
 *                       returns true if the input device was successfully created
 *     emulator_t* emu   : pointer to the emulator
 *     guest_paddr base  : base address of the input device
 *     size_t int_number : interrupt source number if the input device is connected to the PLIC
 */
bool virtio_input_create(emulator_t* emu, guest_paddr base, size_t int_number);

#endif

#endif

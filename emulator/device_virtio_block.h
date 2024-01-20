#ifndef DEVICE_VIRTIO_BLOCK_H
#define DEVICE_VIRTIO_BLOCK_H

#include <stdbool.h>
#include <stdio.h>

#include "emulator.h"
#include "isa.h"

/* virtio_block_create : create and attach a virtio block device to the emulator
 *                       returns true if the block device was successfully created
 *     emulator_t* emu   : pointer to the emulator
 *     guest_paddr base  : base address of the block device
 *     size_t int_number : interrupt source number if the block device is connected to the PLIC
 *     FILE* image       : pointer to an open file storing the raw block device image
 */
bool virtio_block_create(emulator_t* emu, guest_paddr base, size_t int_number, FILE* image);

#endif

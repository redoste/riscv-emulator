#ifndef DEVICE_SYSCON_H
#define DEVICE_SYSCON_H

#include <stdbool.h>

#include "emulator.h"
#include "isa.h"

/* This is an implementation of a simple syscon device
 * it's used by the SBI to power off or reboot the system
 */

/* syscon_create : create and attach a syscon device to the emulator
 *                 returns true if the syscon device was successfully created
 *     emulator_t* emu  : pointer to the emulator
 *     guest_paddr base : base address of the syscon device
 */
bool syscon_create(emulator_t* emu, guest_paddr base);

#endif

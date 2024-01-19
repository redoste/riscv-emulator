#ifndef DEVICE_PLIC_H
#define DEVICE_PLIC_H

#include <stdbool.h>

#include "emulator.h"
#include "isa.h"

/* This is an implementation of a PLIC : Platform-Level Interrupt Controller
 * <https://github.com/riscv/riscv-plic-spec/blob/master/riscv-plic.adoc>
 */

/* plic_create : create and attach a PLIC to the emulator
 *     emulator_t* emu  : pointer to the emulator
 *     guest_paddr base : base address of the PLIC
 */
bool plic_create(emulator_t* emu, guest_paddr base);

/* plic_throw_interrupt : throw and mark a PLIC interrupt as pending
 *     emulator* emu     : pointer to the emulator
 *     size_t int_number : interrupt source number
 */
void plic_throw_interrupt(emulator_t* emu, size_t int_number);

#endif

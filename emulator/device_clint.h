#ifndef DEVICE_CLINT_H
#define DEVICE_CLINT_H

#include <stdbool.h>

#include "emulator.h"
#include "isa.h"

/* This is an implementation of a CLINT : Core Local INTerruptor
 * <https://github.com/riscv/riscv-aclint/blob/main/riscv-aclint.adoc>
 */

/* clint_t : structure storing the current state of the CLINT
 */
typedef struct clint_t clint_t;

/* clint_get_mtime : return the current MTIME counter
 *     const clint_t* clint : pointer to the CLINT
 */
uint64_t clint_get_mtime(const clint_t* clint);

/* clint_create : create and attach a CLINT to the emulator
 *     emulator_t* emu  : pointer to the emulator
 *     guest_paddr base : base address of the CLINT
 */
bool clint_create(emulator_t* emu, guest_paddr base);

#endif

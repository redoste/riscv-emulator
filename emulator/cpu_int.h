#ifndef CPU_INT_H
#define CPU_INT_H

#include <stdint.h>

#include "isa.h"

// NOTE : forward declaration to deal with a cyclic dependency with emulator.h
typedef struct emulator_t emulator_t;

/* cpu_throw_exception : throw an exception and update the CPU state to execute the guest
 *                       trap in the correct privilege mode with updated CSRs
 *     emulator_t* emu        : pointer to the emulator
 *     uint8_t exception_code : exception code that will be written to xcause
 *     guest_reg tval         : trap value that will be written to xtval
 */
void cpu_throw_exception(emulator_t* emu, uint8_t exception_code, guest_reg tval);

/* cpu_mret : handle the mret instruction
 *     emulator_t* emu : pointer to the emulator
 */
void cpu_mret(emulator_t* emu);

/* cpu_sret : handle the sret instruction
 *     emulator_t* emu : pointer to the emulator
 */
void cpu_sret(emulator_t* emu);

/* cpu_wfi : handle the wfi instruction
 *     emulator_t* emu : pointer to the emulator
 */
void cpu_wfi(emulator_t* emu);

/* cpu_check_interrupt : check if an interrupt should be thrown and throw it if required
 *     emulator_t* emu : pointer to the emulator
 */
void cpu_check_interrupt(emulator_t* emu);

#endif

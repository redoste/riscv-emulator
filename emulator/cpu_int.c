#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>

#include "cpu.h"
#include "emulator.h"
#include "isa.h"

void cpu_throw_exception(emulator_t* emu, uint8_t exception_code, guest_reg tval) {
	if (emu->cpu.priv_mode == UO_MODE) {
		fprintf(stderr, "Uncaught exception %d (tval=%016" PRIx64 " PC=%016" PRIx64 ")\n",
			exception_code, tval, emu->cpu.pc);
		abort();
	}

	assert(!emu->cpu.exception_pending);

	// TODO : support exception deleg to S-mode
	assert(!((emu->cpu.csrs.medeleg >> exception_code) & 1));

	emu->cpu.csrs.mcause = (0ll << 63) | (exception_code & 0x3f);
	emu->cpu.csrs.mepc = emu->cpu.pc;
	emu->cpu.csrs.mtval = tval;

	uint8_t mpie = (emu->cpu.csrs.mstatus >> 3) & 1;
	emu->cpu.csrs.mstatus = (emu->cpu.csrs.mstatus & ~((3 << 11) | (1 << 7) | (1 << 3))) |
				((emu->cpu.priv_mode & 3) << 11) |  // MPP
				((mpie & 1) << 7) |                 // MPIE
				(0 << 3);                           // MIE
	emu->cpu.priv_mode = M_MODE;

	// Even in vectored mode, exceptions set PC to the base of xtvec
	emu->cpu.pc = (emu->cpu.csrs.mtvec) & ~3;
	emu->cpu.exception_pending = true;
}
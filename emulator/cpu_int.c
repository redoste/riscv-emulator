#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
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

	if (emu->cpu.priv_mode != M_MODE && ((emu->cpu.csrs.medeleg >> exception_code) & 1)) {
		emu->cpu.csrs.scause = (0ll << 63) | (exception_code & 0x3f);
		emu->cpu.csrs.sepc = emu->cpu.pc;
		emu->cpu.csrs.stval = tval;

		uint8_t spie = (emu->cpu.csrs.mstatus >> 1) & 1;
		emu->cpu.csrs.mstatus = (emu->cpu.csrs.mstatus & ~((1 << 8) | (1 << 5) | (1 << 1))) |
					((emu->cpu.priv_mode & 1) << 8) |  // SPP
					((spie & 1) << 5) |                // SPIE
					(0 << 1);                          // SIE
		emu->cpu.priv_mode = S_MODE;

		// Even in vectored mode, exceptions set PC to the base of xtvec
		emu->cpu.pc = (emu->cpu.csrs.stvec) & ~3;
	} else {
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
	}

	emu->cpu.exception_pending = true;
}

void cpu_mret(emulator_t* emu) {
	if (emu->cpu.priv_mode != M_MODE) {
		cpu_throw_exception(emu, EXC_ILL_INS, 0);
		return;
	}

	uint8_t mpie = (emu->cpu.csrs.mstatus >> 7) & 1;
	uint8_t mpp = (emu->cpu.csrs.mstatus >> 11) & 3;
	assert(mpp == M_MODE || mpp == S_MODE || mpp == U_MODE);
	emu->cpu.csrs.mstatus = (emu->cpu.csrs.mstatus & ~((3 << 11) | (1 << 7) | (1 << 3))) |
				((U_MODE & 3) << 11) |  // MPP
				(1 << 7) |              // MPIE
				((mpie & 1) << 3);      // MIE
	emu->cpu.priv_mode = mpp;
	if (mpp != M_MODE) {
		emu->cpu.csrs.mstatus &= ~(1 << 17);  // MPRV
	}

	emu->cpu.pc = emu->cpu.csrs.mepc;
	emu->cpu.jump_pending = true;
}

void cpu_sret(emulator_t* emu) {
	// TODO : handle mstatus.TSR
	if (emu->cpu.priv_mode != M_MODE && emu->cpu.priv_mode != S_MODE) {
		cpu_throw_exception(emu, EXC_ILL_INS, 0);
		return;
	}

	uint8_t spie = (emu->cpu.csrs.mstatus >> 5) & 1;
	uint8_t spp = (emu->cpu.csrs.mstatus >> 8) & 1;
	assert(spp == S_MODE || spp == U_MODE);
	emu->cpu.csrs.mstatus = (emu->cpu.csrs.mstatus & ~((1 << 17) | (1 << 8) | (1 << 5) | (1 << 1))) |
				(0 << 17) |            // MPRV
				((U_MODE & 1) << 8) |  // SPP
				(1 << 5) |             // SPIE
				((spie & 1) << 1);     // SIE
	emu->cpu.priv_mode = spp;

	emu->cpu.pc = emu->cpu.csrs.sepc;
	emu->cpu.jump_pending = true;
}

void cpu_wfi(emulator_t* emu) {
	if (emu->cpu.priv_mode == UO_MODE ||
	    (emu->cpu.priv_mode != M_MODE && ((emu->cpu.csrs.mstatus >> 21) & 1) /* TW */)) {
		cpu_throw_exception(emu, EXC_ILL_INS, 0);
		return;
	}

	// TODO : Implement a select(2) based WFI, for now this is a nop which is really inefficient
}

static bool cpu_throw_interrupt(emulator_t* emu, size_t interrupt) {
	if (emu->cpu.priv_mode == UO_MODE) {
		fprintf(stderr, "Uncaught interrupt %zu (PC=%016" PRIx64 ")\n",
			interrupt, emu->cpu.pc);
		abort();
	}

	bool deleg = (emu->cpu.csrs.mideleg >> interrupt) & 1;
	bool xie = (emu->cpu.csrs.mie >> interrupt) & 1;
	bool sie = (emu->cpu.csrs.mstatus >> 1) & 1;
	bool mie = (emu->cpu.csrs.mstatus >> 3) & 1;

	privilege_mode_t priv_mode = emu->cpu.priv_mode;

	if (deleg &&
	    ((priv_mode == S_MODE && sie) || (priv_mode < S_MODE)) &&
	    xie) {
		emu->cpu.csrs.scause = (1ll << 63) | (interrupt & 0x3f);
		emu->cpu.csrs.sepc = emu->cpu.pc;

		emu->cpu.csrs.mstatus = (emu->cpu.csrs.mstatus & ~((1 << 8) | (1 << 5) | (1 << 1))) |
					((priv_mode & 1) << 8) |  // SPP
					((sie & 1) << 5) |        // SPIE
					(0 << 1);                 // SIE
		emu->cpu.priv_mode = S_MODE;

		emu->cpu.pc = (emu->cpu.csrs.stvec) & ~3;
		if (emu->cpu.csrs.stvec & 1) {
			emu->cpu.pc += 4 * interrupt;
		}
		return true;
	} else if (!deleg &&
		   ((priv_mode == M_MODE && mie) || (priv_mode < M_MODE)) &&
		   xie) {
		emu->cpu.csrs.mcause = (1ll << 63) | (interrupt & 0x3f);
		emu->cpu.csrs.mepc = emu->cpu.pc;

		emu->cpu.csrs.mstatus = (emu->cpu.csrs.mstatus & ~((3 << 11) | (1 << 7) | (1 << 3))) |
					((priv_mode & 3) << 11) |  // MPP
					((mie & 1) << 7) |         // MPIE
					(0 << 3);                  // MIE
		emu->cpu.priv_mode = M_MODE;

		emu->cpu.pc = (emu->cpu.csrs.mtvec) & ~3;
		if (emu->cpu.csrs.mtvec & 1) {
			emu->cpu.pc += 4 * interrupt;
		}
		return true;
	} else {
		return false;
	}
}

void cpu_check_interrupt(emulator_t* emu) {
	for (size_t i = 0; i < 12; i++) {
		if ((emu->cpu.csrs.mip >> i) & 1) {
			if (cpu_throw_interrupt(emu, i)) {
				return;
			}
		}
	}
}

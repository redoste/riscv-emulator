#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "emulator.h"
#include "isa.h"

guest_reg cpu_csr_read(emulator_t* emu, guest_reg csr_num) {
	privilege_mode_t csr_priv = (csr_num >> 8) & 3;
	if (emu->cpu.priv_mode < csr_priv) {
		cpu_throw_exception(emu, EXC_ILL_INS, 0);
		return 0;
	}

	switch (csr_num) {
#define X_RW(NUM, NAME, MASK, BASE) \
	case (NUM):                 \
		return (emu->cpu.csrs.NAME & (MASK)) | (BASE);
#define X_RW_SHADOW(NUM, SHADOW_NAME, MASK, BASE) \
	case (NUM):                               \
		return (emu->cpu.csrs.SHADOW_NAME & (MASK)) | (BASE);
#define X_RO(NUM, VALUE) \
	case (NUM):      \
		return VALUE;
		X_CSRS
#undef X_RW
#undef X_RW_SHADOW
#undef X_RO
		default:
			cpu_throw_exception(emu, EXC_ILL_INS, 0);
			return 0;
	}
}

void cpu_csr_write(emulator_t* emu, guest_reg csr_num, guest_reg value) {
	privilege_mode_t csr_priv = (csr_num >> 8) & 3;
	if (emu->cpu.priv_mode < csr_priv) {
		cpu_throw_exception(emu, EXC_ILL_INS, 0);
		return;
	}

	switch (csr_num) {
#define X_RW(NUM, NAME, MASK, BASE)                             \
	case (NUM):                                             \
		emu->cpu.csrs.NAME = (value & (MASK)) | (BASE); \
		break;
#define X_RW_SHADOW(NUM, SHADOW_NAME, MASK, BASE)                                   \
	case (NUM):                                                                 \
		emu->cpu.csrs.SHADOW_NAME = (emu->cpu.csrs.SHADOW_NAME & ~(MASK)) | \
					    (value & (MASK)) |                      \
					    (BASE);                                 \
		break;
#define X_RO(NUM, VALUE) \
	case (NUM):      \
		break;
		X_CSRS
#undef X_RW
#undef X_RW_SHADOW
#undef X_RO
		default:
			cpu_throw_exception(emu, EXC_ILL_INS, 0);
			return;
	}
}

guest_reg cpu_csr_exchange(emulator_t* emu, guest_reg csr_num, guest_reg value) {
	guest_reg old_value = cpu_csr_read(emu, csr_num);
	if (!emu->cpu.exception_pending) {
		cpu_csr_write(emu, csr_num, value);
	}
	return old_value;
}

guest_reg cpu_csr_set_bits(emulator_t* emu, guest_reg csr_num, guest_reg mask) {
	guest_reg old_value = cpu_csr_read(emu, csr_num);
	if (!emu->cpu.exception_pending) {
		cpu_csr_write(emu, csr_num, old_value | mask);
	}
	return old_value;
}

guest_reg cpu_csr_clear_bits(emulator_t* emu, guest_reg csr_num, guest_reg mask) {
	guest_reg old_value = cpu_csr_read(emu, csr_num);
	if (!emu->cpu.exception_pending) {
		cpu_csr_write(emu, csr_num, old_value & ~mask);
	}
	return old_value;
}

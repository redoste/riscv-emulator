#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "emulator.h"

guest_reg cpu_csr_read(emulator_t* emu, guest_reg csr_num) {
	switch (csr_num) {
		default:
			fprintf(stderr, "invalid CSR read %016" PRIx64 " PC=%016" PRIx64 "\n",
				csr_num, emu->cpu.pc);
			abort();
	}
}

void cpu_csr_write(emulator_t* emu, guest_reg csr_num, guest_reg value) {
	switch (csr_num) {
		default:
			(void)value;
			fprintf(stderr, "invalid CSR write %016" PRIx64 " PC=%016" PRIx64 "\n",
				csr_num, emu->cpu.pc);
			abort();
	}
}

guest_reg cpu_csr_exchange(emulator_t* emu, guest_reg csr_num, guest_reg value) {
	guest_reg old_value = cpu_csr_read(emu, csr_num);
	cpu_csr_write(emu, csr_num, value);
	return old_value;
}

guest_reg cpu_csr_set_bits(emulator_t* emu, guest_reg csr_num, guest_reg mask) {
	guest_reg old_value = cpu_csr_read(emu, csr_num);
	cpu_csr_write(emu, csr_num, old_value | mask);
	return old_value;
}

guest_reg cpu_csr_clear_bits(emulator_t* emu, guest_reg csr_num, guest_reg mask) {
	guest_reg old_value = cpu_csr_read(emu, csr_num);
	cpu_csr_write(emu, csr_num, old_value & ~mask);
	return old_value;
}

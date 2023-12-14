#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "emulator.h"

guest_reg cpu_read_csr(emulator_t* emu, guest_reg csr_num) {
	switch (csr_num) {
		default:
			fprintf(stderr, "invalid CSR read %016" PRIx64 " PC=%016" PRIx64 "\n",
				csr_num, emu->cpu.pc);
			abort();
	}
}

void cpu_write_csr(emulator_t* emu, guest_reg csr_num, guest_reg value) {
	switch (csr_num) {
		default:
			(void)value;
			fprintf(stderr, "invalid CSR write %016" PRIx64 " PC=%016" PRIx64 "\n",
				csr_num, emu->cpu.pc);
			abort();
	}
}

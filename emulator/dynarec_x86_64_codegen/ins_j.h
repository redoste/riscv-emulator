#ifndef INS_J
#define INS_J

#include "codegen.h"

C_J(JAL) {
	S_J();

	if (!rd_zero) {
		A(MOV, OP_REG(RAX), OP_REG(R9));
		A(ADD, OP_REG(RAX), OP_IMM(4));
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	}

	A_IMM(ADD, OP_REG(R9), OP_RELOC_IMM32);
	E_J();
}

#endif

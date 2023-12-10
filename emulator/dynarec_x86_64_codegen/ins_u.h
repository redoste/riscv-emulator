#ifndef INS_U
#define INS_U

#include "codegen.h"

C_U(AUIPC) {
	S_U();

	if (rd_zero) {
		E();
	}

	A_IMM(MOV, OP_REG(RAX), OP_RELOC_IMM32);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A(ADD, OP_REG(RAX), OP_REG(R9));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_U(LUI) {
	S_U();

	if (rd_zero) {
		E();
	}

	A_IMM(MOV, OP_REG(RAX), OP_RELOC_IMM32);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

#endif

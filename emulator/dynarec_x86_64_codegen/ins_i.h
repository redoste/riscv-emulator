#ifndef INS_I_H
#define INS_I_H

#include "codegen.h"

C_I(LB) {}
C_I(LH) {}
C_I(LW) {}
C_I(LD) {}
C_I(LBU) {}
C_I(LHU) {}
C_I(LWU) {}

C_I(FENCE) {}

C_I(ADDI) {
	S_I();

	if (rd_zero) {
		E();
	}

	if (rs1_zero) {
		A_IMM(MOV, OP_REG(RAX), OP_RELOC_IMM32);
		A(MOVSX, OP_REG(RAX), OP_REG(RAX));
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	} else {
		A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
		A_IMM(ADD, OP_REG(RAX), OP_RELOC_IMM32);
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	}
	E();
}

C_I(SLLI) {
	S_I();

	if (rd_zero) {
		E();
	}

	if (rs1_zero) {
		A_RD(MOV, OP_RELOC_RV_REG, OP_IMM(0));
	} else {
		A_IMM(MOV, OP_REG(RCX), OP_RELOC_IMM32);
		A_RD(SHL, OP_RELOC_RV_REG, 0);
	}
	E();
}

C_I(SLTI) {}
C_I(SLTIU) {}
C_I(XORI) {}
C_I(SRLI) {}
C_I(ORI) {}
C_I(ANDI) {}

C_I(ADDIW) {}
C_I(SLLIW) {}
C_I(SRLIW) {}

C_I(JALR) {}

C_I(ECALL) {}

#endif

#ifndef INS_R_H
#define INS_R_H

#include "codegen.h"

C_R(ADD) {
	S_R();

	if (rd_zero) {
		E();
	}

	if (rs1_zero && rs2_zero) {
		A_RD(MOV, OP_RELOC_RV_REG, OP_IMM(0));
	} else if (rs1_zero) {
		A_RS2(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	} else if (rs2_zero) {
		A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	} else {
		A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
		A_RS2(ADD, OP_REG(RAX), OP_RELOC_RV_REG);
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	}
	E();
}

C_R(SUB) {}
C_R(SLL) {}
C_R(SLT) {}
C_R(SLTU) {}
C_R(XOR) {}
C_R(SRL) {}
C_R(SRA) {}
C_R(OR) {}
C_R(AND) {}

C_R(ADDW) {}
C_R(SUBW) {}
C_R(SLLW) {}
C_R(SRLW) {}
C_R(SRAW) {}

C_R(MUL) {}
C_R(MULH) {}
C_R(MULHSU) {}
C_R(MULHU) {}
C_R(DIV) {}
C_R(DIVU) {}
C_R(REM) {}
C_R(REMU) {}

C_R(MULW) {}
C_R(DIVW) {}
C_R(DIVUW) {}
C_R(REMW) {}
C_R(REMUW) {}

#endif

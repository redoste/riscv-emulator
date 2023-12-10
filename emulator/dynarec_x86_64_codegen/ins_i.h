#ifndef INS_I_H
#define INS_I_H

#include "codegen.h"

// TODO : rs1_zero optimizations

C_I(LB) {}
C_I(LH) {}
C_I(LW) {}
C_I(LD) {}
C_I(LBU) {}
C_I(LHU) {}
C_I(LWU) {}

C_I(FENCE) {
	S_I();
	E();
}

C_I(ADDI) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_IMM(ADD, OP_RELOC_IMM32, 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_I(SLLI) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_IMM(MOV, OP_REG(RCX), OP_RELOC_IMM32);
	A(AND, OP_REG(RCX), OP_IMM(0x3f));
	A(SHL, OP_REG(RAX), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_I(SLTI) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A(XOR, OP_REG(RCX), OP_REG(RCX));
	A_IMM(CMP, OP_RELOC_IMM32, 0);
	A(SETL, OP_REG(RCX), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RCX));

	E();
}

C_I(SLTIU) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A(XOR, OP_REG(RCX), OP_REG(RCX));
	A_IMM(CMP, OP_RELOC_IMM32, 0);
	A(SETB, OP_REG(RCX), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RCX));

	E();
}

C_I(XORI) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_IMM(XOR, OP_RELOC_IMM32, 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

C_I(SRLI) {
	S_I();

	if (rd_zero) {
		E();
	}

	// TODO : do this check at dynarec time
	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_IMM(MOV, OP_REG(RCX), OP_RELOC_IMM32);
	A(TEST, OP_REG(RCX), OP_IMM(0x400));
	A(JZ, OP_IMM(12), 0);

	A(AND, OP_REG(RCX), OP_IMM(0x3f));
	A(SAR, OP_REG(RAX), 0);
	A(JMP, OP_IMM(10), 0);

	A(AND, OP_REG(RCX), OP_IMM(0x3f));
	A(SHR, OP_REG(RAX), 0);

	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

C_I(ORI) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_IMM(OR, OP_RELOC_IMM32, 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

C_I(ANDI) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_IMM(AND, OP_RELOC_IMM32, 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

C_I(ADDIW) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOVZX, OP_REG(RAX), OP_RELOC_RV_REG);
	A_IMM(ADD, OP_RELOC_IMM32, 0);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

C_I(SLLIW) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOVZX, OP_REG(RAX), OP_RELOC_RV_REG);
	A_IMM(MOV, OP_REG(RCX), OP_RELOC_IMM32);
	A(AND, OP_REG(RCX), OP_IMM(0x1f));
	A(SHL, OP_REG(RAX), 0);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_I(SRLIW) {
	S_I();

	if (rd_zero) {
		E();
	}

	// TODO : do this check at dynarec time
	A_RS1(MOVZX, OP_REG(RAX), OP_RELOC_RV_REG);
	A_IMM(MOV, OP_REG(RCX), OP_RELOC_IMM32);
	A(TEST, OP_REG(RCX), OP_IMM(0x400));
	A(JZ, OP_IMM(15), 0);

	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A(AND, OP_REG(RCX), OP_IMM(0x1f));
	A(SAR, OP_REG(RAX), 0);
	A(JMP, OP_IMM(13), 0);

	A(AND, OP_REG(RCX), OP_IMM(0x1f));
	A(SHR, OP_REG(RAX), 0);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));

	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

C_I(JALR) {
	S_I();

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_IMM(ADD, OP_RELOC_IMM32, 0);
	A(AND, OP_IMM(0xfffffffffffffffe), 0);

	if (!rd_zero) {
		A(MOV, OP_REG(RCX), OP_REG(R9));
		A(ADD, OP_REG(RCX), OP_IMM(4));
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RCX));
	}

	A(MOV, OP_REG(R9), OP_REG(RAX));
	E_J();
}

C_I(ECALL) {}

#endif

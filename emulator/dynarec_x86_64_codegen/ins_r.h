#ifndef INS_R_H
#define INS_R_H

#include "codegen.h"

// TODO : rs1_zero & rs2_zero optimizations

C_R(ADD) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(ADD, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(SUB) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(SUB, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(SLL) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RCX), OP_RELOC_RV_REG);
	A(AND, OP_REG(RCX), OP_IMM(0x3f));
	A(SHL, OP_REG(RAX), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(SLT) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A(XOR, OP_REG(RCX), OP_REG(RCX));
	A_RS2(CMP, OP_REG(RAX), OP_RELOC_RV_REG);
	A(SETL, OP_REG(RCX), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RCX));
	E();
}

C_R(SLTU) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A(XOR, OP_REG(RCX), OP_REG(RCX));
	A_RS2(CMP, OP_REG(RAX), OP_RELOC_RV_REG);
	A(SETB, OP_REG(RCX), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RCX));
	E();
}

C_R(XOR) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(XOR, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(SRL) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RCX), OP_RELOC_RV_REG);
	A(AND, OP_REG(RCX), OP_IMM(0x3f));
	A(SHR, OP_REG(RAX), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(SRA) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RCX), OP_RELOC_RV_REG);
	A(AND, OP_REG(RCX), OP_IMM(0x3f));
	A(SAR, OP_REG(RAX), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(OR) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(OR, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(AND) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(AND, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(ADDW) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOVZX, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(ADD, OP_REG(RAX), OP_RELOC_RV_REG);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(SUBW) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOVZX, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(SUB, OP_REG(RAX), OP_RELOC_RV_REG);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(SLLW) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOVZX, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RCX), OP_RELOC_RV_REG);
	A(AND, OP_REG(RCX), OP_IMM(0x1f));
	A(SHL, OP_REG(RAX), 0);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(SRLW) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOVZX, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RCX), OP_RELOC_RV_REG);
	A(AND, OP_REG(RCX), OP_IMM(0x1f));
	A(SHR, OP_REG(RAX), 0);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(SRAW) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOVSX, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RCX), OP_RELOC_RV_REG);
	A(AND, OP_REG(RCX), OP_IMM(0x1f));
	A(SAR, OP_REG(RAX), 0);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(MUL) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RDX), OP_RELOC_RV_REG);
	A(MUL, OP_REG(RDX), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(MULH) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RDX), OP_RELOC_RV_REG);
	A(IMUL, OP_REG(RDX), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RDX));
	E();
}

C_R(MULHSU) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RSI), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RDI), OP_RELOC_RV_REG);

	// rcx = (rs1 (unsigned*) rs2) >> 64
	A(MOV, OP_REG(RAX), OP_REG(RSI));
	A(MUL, OP_REG(RDI), 0);
	A(MOV, OP_REG(RCX), OP_REG(RDX));

	// rcx += ((rs1 >> 63) (signed*) rs2)
	A(MOV, OP_REG(RAX), OP_REG(RSI));
	A(SAR, OP_REG(RAX), OP_IMM(63));
	A(IMUL, OP_REG(RDI), 0);
	A(ADD, OP_REG(RCX), OP_REG(RAX));

	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RCX));

	E();
}

C_R(MULHU) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RDX), OP_RELOC_RV_REG);
	A(MUL, OP_REG(RDX), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RDX));
	E();
}

C_R(DIV) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RSI), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RDI), OP_RELOC_RV_REG);

	// if (rs2 == 0)
	A(TEST, OP_REG(RDI), OP_REG(RDI));
	A(JZ, OP_IMM(33), 0);

	// if (rs2 == -1) and ...
	A(CMP, OP_REG(RDI), OP_IMM(-1));
	A(JNZ, OP_IMM(14), 0);

	// ... (rs1 == INT64_MIN)
	A(MOV, OP_REG(RAX), OP_IMM(1));
	A(SHL, OP_REG(RAX), OP_IMM(63));
	A(CMP, OP_REG(RSI), OP_REG(RAX));
	A(JZ, OP_IMM(16), 0);

	A(MOV, OP_REG(RAX), OP_REG(RSI));
	A(CQO, 0, 0);
	A(IDIV, OP_REG(RDI), 0);
	A(JMP, OP_IMM(6), 0);

	A(OR, OP_IMM(-1), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(DIVU) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RDI), OP_RELOC_RV_REG);

	// if (rs2 == 0)
	A(TEST, OP_REG(RDI), OP_REG(RDI));
	A(JZ, OP_IMM(8), 0);

	A(XOR, OP_REG(RDX), OP_REG(RDX));
	A(DIV, OP_REG(RDI), 0);
	A(JMP, OP_IMM(6), 0);

	A(OR, OP_IMM(-1), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(REM) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RSI), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RDI), OP_RELOC_RV_REG);

	// if (rs2 == 0)
	A(TEST, OP_REG(RDI), OP_REG(RDI));
	A(JZ, OP_IMM(37), 0);

	// if (rs2 == -1) and ...
	A(CMP, OP_REG(RDI), OP_IMM(-1));
	A(JNZ, OP_IMM(14), 0);

	// ... (rs1 == INT64_MIN)
	A(MOV, OP_REG(RAX), OP_IMM(1));
	A(SHL, OP_REG(RAX), OP_IMM(63));
	A(CMP, OP_REG(RSI), OP_REG(RAX));
	A(JZ, OP_IMM(10), 0);

	A(MOV, OP_REG(RAX), OP_REG(RSI));
	A(CQO, 0, 0);
	A(IDIV, OP_REG(RDI), 0);
	A(JMP, OP_IMM(7), 0);

	A(SHL, OP_REG(RSI), OP_IMM(1));
	A(MOV, OP_REG(RDX), OP_REG(RSI));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RDX));
	E();
}

C_R(REMU) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOV, OP_REG(RDI), OP_RELOC_RV_REG);

	// if (rs2 == 0)
	A(TEST, OP_REG(RDI), OP_REG(RDI));
	A(JZ, OP_IMM(8), 0);

	A(XOR, OP_REG(RDX), OP_REG(RDX));
	A(DIV, OP_REG(RDI), 0);
	A(JMP, OP_IMM(3), 0);

	A(MOV, OP_REG(RDX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RDX));
	E();
}

C_R(MULW) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOVSX, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOVSX, OP_REG(RDX), OP_RELOC_RV_REG);
	A(MUL, OP_REG(RDX), 0);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(DIVW) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOVSX, OP_REG(RSI), OP_RELOC_RV_REG);
	A_RS2(MOVSX, OP_REG(RDI), OP_RELOC_RV_REG);

	// if (rs2 == 0)
	A(TEST, OP_REG(RDI), OP_REG(RDI));
	A(JZ, OP_IMM(39), 0);

	// if (rs2 == -1) and ...
	A(CMP, OP_REG(RDI), OP_IMM(-1));
	A(JNZ, OP_IMM(17), 0);

	// ... (rs1 == INT32_MIN)
	A(MOV, OP_REG(RAX), OP_IMM(1));
	A(SHL, OP_REG(RAX), OP_IMM(31));
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A(CMP, OP_REG(RSI), OP_REG(RAX));
	A(JZ, OP_IMM(19), 0);

	A(MOV, OP_REG(RAX), OP_REG(RSI));
	A(CQO, 0, 0);
	A(IDIV, OP_REG(RDI), 0);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A(JMP, OP_IMM(6), 0);

	A(OR, OP_IMM(-1), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(DIVUW) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOVZX, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOVZX, OP_REG(RDI), OP_RELOC_RV_REG);

	// if (rs2 == 0)
	A(TEST, OP_REG(RDI), OP_REG(RDI));
	A(JZ, OP_IMM(11), 0);

	A(XOR, OP_REG(RDX), OP_REG(RDX));
	A(DIV, OP_REG(RDI), 0);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A(JMP, OP_IMM(6), 0);

	A(OR, OP_IMM(-1), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	E();
}

C_R(REMW) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOVSX, OP_REG(RSI), OP_RELOC_RV_REG);
	A_RS2(MOVSX, OP_REG(RDI), OP_RELOC_RV_REG);

	// if (rs2 == 0)
	A(TEST, OP_REG(RDI), OP_REG(RDI));
	A(JZ, OP_IMM(42), 0);

	// if (rs2 == -1) and ...
	A(CMP, OP_REG(RDI), OP_IMM(-1));
	A(JNZ, OP_IMM(17), 0);

	// ... (rs1 == INT32_MIN)
	A(MOV, OP_REG(RAX), OP_IMM(1));
	A(SHL, OP_REG(RAX), OP_IMM(31));
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A(CMP, OP_REG(RSI), OP_REG(RAX));
	A(JZ, OP_IMM(13), 0);

	A(MOV, OP_REG(RAX), OP_REG(RSI));
	A(CQO, 0, 0);
	A(IDIV, OP_REG(RDI), 0);
	A(MOVSX, OP_REG(RDX), OP_REG(RDX));
	A(JMP, OP_IMM(6), 0);

	A(XOR, OP_REG(RSI), OP_REG(RSI));
	A(MOV, OP_REG(RDX), OP_REG(RSI));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RDX));
	E();
}

C_R(REMUW) {
	S_R();

	if (rd_zero) {
		E();
	}

	A_RS1(MOVZX, OP_REG(RAX), OP_RELOC_RV_REG);
	A_RS2(MOVZX, OP_REG(RDI), OP_RELOC_RV_REG);

	// if (rs2 == 0)
	A(TEST, OP_REG(RDI), OP_REG(RDI));
	A(JZ, OP_IMM(11), 0);

	A(XOR, OP_REG(RDX), OP_REG(RDX));
	A(DIV, OP_REG(RDI), 0);
	A(MOVSX, OP_REG(RDX), OP_REG(RDX));
	A(JMP, OP_IMM(3), 0);

	A(MOV, OP_REG(RDX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RDX));
	E();
}

C_R(LR_W) {}
C_R(SC_W) {}
C_R(AMOSWAP_W) {}
C_R(AMOADD_W) {}
C_R(AMOXOR_W) {}
C_R(AMOAND_W) {}
C_R(AMOOR_W) {}
C_R(AMOMIN_W) {}
C_R(AMOMAX_W) {}
C_R(AMOMINU_W) {}
C_R(AMOMAXU_W) {}

C_R(LR_D) {}
C_R(SC_D) {}
C_R(AMOSWAP_D) {}
C_R(AMOADD_D) {}
C_R(AMOXOR_D) {}
C_R(AMOAND_D) {}
C_R(AMOOR_D) {}
C_R(AMOMIN_D) {}
C_R(AMOMAX_D) {}
C_R(AMOMINU_D) {}
C_R(AMOMAXU_D) {}

#endif

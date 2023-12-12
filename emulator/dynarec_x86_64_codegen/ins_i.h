#ifndef INS_I_H
#define INS_I_H

#include "codegen.h"

// TODO : rs1_zero optimizations

C_I(LB) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RSI), OP_RELOC_RV_REG);
	A_IMM(ADD, OP_REG(RSI), OP_RELOC_IMM32);
	EMU_FUNCTION(4);
	A(MOVSX8, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

C_I(LH) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RSI), OP_RELOC_RV_REG);
	A_IMM(ADD, OP_REG(RSI), OP_RELOC_IMM32);
	EMU_FUNCTION(5);
	A(MOVSX16, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

C_I(LW) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RSI), OP_RELOC_RV_REG);
	A_IMM(ADD, OP_REG(RSI), OP_RELOC_IMM32);
	EMU_FUNCTION(6);
	A(MOVSX, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

C_I(LD) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RSI), OP_RELOC_RV_REG);
	A_IMM(ADD, OP_REG(RSI), OP_RELOC_IMM32);
	EMU_FUNCTION(7);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

C_I(LBU) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RSI), OP_RELOC_RV_REG);
	A_IMM(ADD, OP_REG(RSI), OP_RELOC_IMM32);
	EMU_FUNCTION(4);
	A(AND, OP_IMM(0xff), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

C_I(LHU) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RSI), OP_RELOC_RV_REG);
	A_IMM(ADD, OP_REG(RSI), OP_RELOC_IMM32);
	EMU_FUNCTION(5);
	A(AND, OP_IMM(0xffff), 0);
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

C_I(LWU) {
	S_I();

	if (rd_zero) {
		E();
	}

	A_RS1(MOV, OP_REG(RSI), OP_RELOC_RV_REG);
	A_IMM(ADD, OP_REG(RSI), OP_RELOC_IMM32);
	EMU_FUNCTION(6);
	A(MOVZX, OP_REG(RAX), OP_REG(RAX));
	A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));

	E();
}

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

C_I(ECALL) {
	S_I();

	A_IMM(MOV, OP_REG(RAX), OP_RELOC_IMM32);
	A(CMP, OP_IMM(1), 0);
	A(JZ, OP_IMM(6), 0);
	A(MOV, OP_REG(RAX), OP_DISP(R11, 8 * 8));
	A(JMP, OP_IMM(4), 0);
	A(MOV, OP_REG(RAX), OP_DISP(R11, 9 * 8));

	/* We don't use the EMU_FUNCTION macro to save space by not generating two times
	 * the PUSHs and POPs
	 */
	A(PUSH, OP_REG(R8), 0);
	A(PUSH, OP_REG(R9), 0);
	A(PUSH, OP_REG(R10), 0);
	A(PUSH, OP_REG(R11), 0);
	A(MOV, OP_REG(RDI), OP_REG(R12));
	A(CALL, OP_REG(RAX), 0);
	A(POP, OP_REG(R11), 0);
	A(POP, OP_REG(R10), 0);
	A(POP, OP_REG(R9), 0);
	A(POP, OP_REG(R8), 0);

	E();
}

C_I(CSRRW) {}
C_I(CSRRS) {}
C_I(CSRRC) {}
C_I(CSRRWI) {}
C_I(CSRRSI) {}
C_I(CSRRCI) {}

#endif

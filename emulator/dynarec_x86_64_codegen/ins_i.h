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

C_I(FENCEI) {
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

C_I_IMM_F12(SYSTEM) {
	S_I_IMM();

	if (f12 == F12_EBREAK) {
		EMU_FUNCTION(9);
	} else if (f12 == F12_ECALL) {
		EMU_FUNCTION(8);
	} else if (f12 == F12_MRET) {
		EMU_FUNCTION(15);
	} else if (f12 == F12_WFI) {
		EMU_FUNCTION(-1);
	} else {
		abort();
	}

	E();
}

C_I_IMM_F7(SYSTEM) {
	S_I_IMM();

	if (f7 == F7_SFENCE_VMA) {
		E();
	} else {
		abort();
	}
}

C_I(CSRRW) {
	S_I();

	if (rs1_zero) {
		A(XOR, OP_REG(RDX), OP_REG(RDX));
	} else {
		A_RS1(MOV, OP_REG(RDX), OP_RELOC_RV_REG);
	}

	A_IMM(MOV, OP_REG(RSI), OP_RELOC_IMM32);
	A(AND, OP_REG(RSI), OP_IMM(CSR_MASK));

	if (rd_zero) {
		EMU_FUNCTION(11);
	} else {
		EMU_FUNCTION(12);
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	}

	E();
}

C_I(CSRRS) {
	S_I();

	if (rs1_zero && rd_zero) {
		E();
	}

	A_IMM(MOV, OP_REG(RSI), OP_RELOC_IMM32);
	A(AND, OP_REG(RSI), OP_IMM(CSR_MASK));

	if (rs1_zero && !rd_zero) {
		EMU_FUNCTION(10);
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	} else {
		A_RS1(MOV, OP_REG(RDX), OP_RELOC_RV_REG);
		EMU_FUNCTION(13);
		if (!rd_zero) {
			A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
		}
	}

	E();
}

C_I(CSRRC) {
	S_I();

	if (rs1_zero && rd_zero) {
		E();
	}

	A_IMM(MOV, OP_REG(RSI), OP_RELOC_IMM32);
	A(AND, OP_REG(RSI), OP_IMM(CSR_MASK));

	if (rs1_zero && !rd_zero) {
		EMU_FUNCTION(10);
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	} else {
		A_RS1(MOV, OP_REG(RDX), OP_RELOC_RV_REG);
		EMU_FUNCTION(14);
		if (!rd_zero) {
			A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
		}
	}

	E();
}

C_I(CSRRWI) {
	S_I();

	if (rs1_zero) {
		A(XOR, OP_REG(RDX), OP_REG(RDX));
	} else {
		A_RS1UIMM(MOV, OP_REG(RDX), OP_RELOC_IMM32);
	}

	A_IMM(MOV, OP_REG(RSI), OP_RELOC_IMM32);
	A(AND, OP_REG(RSI), OP_IMM(CSR_MASK));

	if (rd_zero) {
		EMU_FUNCTION(11);
	} else {
		EMU_FUNCTION(12);
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	}

	E();
}

C_I(CSRRSI) {
	S_I();

	if (rs1_zero && rd_zero) {
		E();
	}

	A_IMM(MOV, OP_REG(RSI), OP_RELOC_IMM32);
	A(AND, OP_REG(RSI), OP_IMM(CSR_MASK));

	if (rs1_zero && !rd_zero) {
		EMU_FUNCTION(10);
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	} else {
		A_RS1UIMM(MOV, OP_REG(RDX), OP_RELOC_IMM32);
		EMU_FUNCTION(13);
		if (!rd_zero) {
			A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
		}
	}

	E();
}

C_I(CSRRCI) {
	S_I();

	if (rs1_zero && rd_zero) {
		E();
	}

	A_IMM(MOV, OP_REG(RSI), OP_RELOC_IMM32);
	A(AND, OP_REG(RSI), OP_IMM(CSR_MASK));

	if (rs1_zero && !rd_zero) {
		EMU_FUNCTION(10);
		A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
	} else {
		A_RS1UIMM(MOV, OP_REG(RDX), OP_RELOC_IMM32);
		EMU_FUNCTION(14);
		if (!rd_zero) {
			A_RD(MOV, OP_RELOC_RV_REG, OP_REG(RAX));
		}
	}

	E();
}

#endif

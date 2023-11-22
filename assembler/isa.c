#include "isa.h"

const char* const INS_NAMES[INS_COUNT] = {
#define X_R(MNEMONIC, O, F3, F7) [INS_##MNEMONIC] = #MNEMONIC,
#define X_I(MNEMONIC, O, F3)     [INS_##MNEMONIC] = #MNEMONIC,
#define X_I_S(MNEMONIC, O, F3)   [INS_##MNEMONIC] = #MNEMONIC,
#define X_S(MNEMONIC, O, F3)     [INS_##MNEMONIC] = #MNEMONIC,
#define X_B(MNEMONIC, O, F3)     [INS_##MNEMONIC] = #MNEMONIC,
#define X_U(MNEMONIC, O)         [INS_##MNEMONIC] = #MNEMONIC,
#define X_J(MNEMONIC, O)         [INS_##MNEMONIC] = #MNEMONIC,
#define X_P(MNEMONIC)            [INS_##MNEMONIC] = #MNEMONIC,
	X_INSTRUCTIONS
#undef X_R
#undef X_I
#undef X_I_S
#undef X_S
#undef X_B
#undef X_U
#undef X_J
#undef X_P
};

const char* const REG_ALIAS[REG_COUNT] = {
	[0] = "zero",
	[1] = "ra",
	[2] = "sp",
	[3] = "gp",
	[4] = "tp",
	[5] = "t0",
	[6] = "t1",
	[7] = "t2",
	[8] = "s0",
	[9] = "s1",
	[10] = "a0",
	[11] = "a1",
	[12] = "a2",
	[13] = "a3",
	[14] = "a4",
	[15] = "a5",
	[16] = "a6",
	[17] = "a7",
	[18] = "s2",
	[19] = "s3",
	[20] = "s4",
	[21] = "s5",
	[22] = "s6",
	[23] = "s7",
	[24] = "s8",
	[25] = "s9",
	[26] = "s10",
	[27] = "s11",
	[28] = "t3",
	[29] = "t4",
	[30] = "t5",
	[31] = "t6",
};

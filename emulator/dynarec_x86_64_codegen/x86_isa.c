#include <string.h>

#include "x86_isa.h"

#define EOL \
	{ 0, 0, 0, 0, 0, 0 }
#define L(m) static const x86_ins_encoding_t x86_##m##_encodings[] =
#define O(n) X86_OPERAND_ENCODING_##n

/* NOTE : We sort the encoding types by priority of usage
 *        e.g. we put the version with 8 bit immediates before the version with
 *        32 bit immediates
 */

L(MOV){
	{0x89, 1, 0, true, O(RM64), O(R64)},
	{0x8B, 1, 0, true, O(R64), O(RM64)},
	{0xB8, 1, 0, false, O(R64), O(IMM32)},
	{0xC7, 1, 0, true, O(RM64), O(IMM32)},
	EOL,
};

L(MOVSX){
	{0x63, 1, 0, true, O(R64), O(RM64)},
	EOL,
};

L(ADD){
	{0x01, 1, 0, true, O(RM64), O(R64)},
	{0x03, 1, 0, true, O(R64), O(RM64)},
	{0x83, 1, 0, true, O(RM64), O(IMM8)},
	{0x81, 1, 0, true, O(RM64), O(IMM32)},
	EOL,
};

L(SUB){
	{0x29, 1, 0, true, O(RM64), O(R64)},
	EOL,
};

L(SHL){
	/* TODO : support hardcoded registers (here `SHL r/m64, CL`)
	 *        we use NONE as a substitute for now
	 */
	{0xD3, 1, 4, true, O(RM64), O(NONE)},
	EOL,
};

L(MUL){
	{0xF7, 1, 4, true, O(RM64), O(NONE)},
	EOL,
};

L(JMP){
	{0xFF, 1, 4, false, O(RM64), O(NONE)},
	{0xEB, 1, 0, false, O(IMM8), O(NONE)},
	EOL,
};

#undef O
#undef L
#undef EOL

const x86_ins_encoding_t* const X86_INS_ENCODINGS[] = {
#define X(MNEMONIC) [X86_MNEMONIC_##MNEMONIC] = x86_##MNEMONIC##_encodings,
	X_X86_MNEMONICS
#undef X
};

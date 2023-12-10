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

/* NOTE : MOVSX16 and MOVSX8 are technically MOVSX, but the assembler still doesn't know the
 *        difference between RAX, EAX and AL, so we hardcode it using a different mnemonic
 */
L(MOVSX16){
	{0xBF0F, 2, 0, true, O(R64), O(RM64)},
	EOL,
};

L(MOVSX8){
	{0xBE0F, 2, 0, true, O(R64), O(RM64)},
	EOL,
};

L(MOVZX){
	/* NOTE : this is technically a 32-bit MOV, but the upper half of the register
	 *        is zeroed, so it behaves like a MOVZX
	 *        we use it this way to make it clearer and to not complexify the logic
	 *        of the assembler (it doesn't know the difference between RAX, EAX and AL)
	 */
	{0x8B, 1, 0, false, O(R64), O(RM64)},  // MOV r32, r/m32
	EOL,
};

L(ADD){
	{0x01, 1, 0, true, O(RM64), O(R64)},
	{0x03, 1, 0, true, O(R64), O(RM64)},
	{0x83, 1, 0, true, O(RM64), O(IMM8)},
	{0x81, 1, 0, true, O(RM64), O(IMM32)},
	{0x05, 1, 0, true, O(IMM32), O(NONE)},  // ADD RAX, imm32
	EOL,
};

L(SUB){
	{0x29, 1, 0, true, O(RM64), O(R64)},
	{0x2B, 1, 0, true, O(R64), O(RM64)},
	EOL,
};

L(NEG){
	{0xF7, 1, 3, true, O(RM64), O(NONE)},
	EOL,
};

L(SHL){
	{0xC1, 1, 4, true, O(RM64), O(IMM8)},
	/* TODO : support hardcoded registers (here `SHL r/m64, CL`)
	 *        we use NONE as a substitute for now
	 */
	{0xD3, 1, 4, true, O(RM64), O(NONE)},  // SHL r/m64, CL
	EOL,
};

L(SHR){
	{0xD3, 1, 5, true, O(RM64), O(NONE)},  // SHR r/m64, CL
	EOL,
};

L(SAR){
	{0xC1, 1, 7, true, O(RM64), O(IMM8)},
	{0xD3, 1, 7, true, O(RM64), O(NONE)},  // SAR r/m64, CL
	EOL,
};

L(XOR){
	{0x31, 1, 0, true, O(RM64), O(R64)},
	{0x33, 1, 0, true, O(R64), O(RM64)},
	{0x35, 1, 0, true, O(IMM32), O(NONE)},  // XOR RAX, imm32
	EOL,
};

L(OR){
	{0x09, 1, 0, true, O(RM64), O(R64)},
	{0x0B, 1, 0, true, O(R64), O(RM64)},
	{0x0D, 1, 0, true, O(IMM32), O(NONE)},  // OR RAX, imm32
	EOL,
};

L(AND){
	{0x21, 1, 0, true, O(RM64), O(R64)},
	{0x23, 1, 0, true, O(R64), O(RM64)},
	{0x81, 1, 4, true, O(RM64), O(IMM32)},
	{0x25, 1, 0, true, O(IMM32), O(NONE)},  // AND RAX, imm32
	EOL,
};

L(CMP){
	{0x39, 1, 0, true, O(RM64), O(R64)},
	{0x3B, 1, 0, true, O(R64), O(RM64)},
	{0x81, 1, 7, true, O(RM64), O(IMM32)},
	{0x3D, 1, 0, true, O(IMM32), O(NONE)},  // CMP RAX, imm32
	EOL,
};

L(TEST){
	{0xF7, 1, 0, true, O(RM64), O(IMM32)},
	{0x85, 1, 0, true, O(RM64), O(R64)},
	{0xA9, 1, 0, true, O(IMM32), O(NONE)},  // TEST RAX, imm32
	EOL,
};

L(SETL){
	{0x9C0F, 2, 0, false, O(RM64), O(NONE)},
	EOL,
};

L(SETB){
	{0x920F, 2, 0, false, O(RM64), O(NONE)},
	EOL,
};

L(MUL){
	{0xF7, 1, 4, true, O(RM64), O(NONE)},
	EOL,
};

L(IMUL){
	{0xF7, 1, 5, true, O(RM64), O(NONE)},
	EOL,
};

L(CQO){
	{0x99, 1, 0, true, O(NONE), O(NONE)},
	EOL,
};

L(DIV){
	{0xF7, 1, 6, true, O(RM64), O(NONE)},
	EOL,
};

L(IDIV){
	{0xF7, 1, 7, true, O(RM64), O(NONE)},
	EOL,
};

L(JZ){
	{0x74, 1, 0, false, O(IMM8), O(NONE)},
	EOL,
};

L(JNZ){
	{0x75, 1, 0, false, O(IMM8), O(NONE)},
	EOL,
};

L(JL){
	{0x7C, 1, 0, false, O(IMM8), O(NONE)},
	EOL,
};

L(JGE){
	{0x7D, 1, 0, false, O(IMM8), O(NONE)},
	EOL,
};

L(JNC){
	{0x73, 1, 0, false, O(IMM8), O(NONE)},
	EOL,
};

L(JC){
	{0x72, 1, 0, false, O(IMM8), O(NONE)},
	EOL,
};

L(JMP){
	{0xFF, 1, 4, false, O(RM64), O(NONE)},
	{0xEB, 1, 0, false, O(IMM8), O(NONE)},
	EOL,
};

L(CALL){
	{0xFF, 1, 2, false, O(RM64), O(NONE)},
	EOL,
};

L(PUSH){
	{0x50, 1, 0, false, O(R64), O(NONE)},
	{0xFF, 1, 6, false, O(RM64), O(NONE)},
	EOL,
};

L(POP){
	{0x58, 1, 0, false, O(R64), O(NONE)},
	{0x8F, 1, 0, false, O(RM64), O(NONE)},
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

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#include "isa.h"

#include "ins_b.h"
#include "ins_i.h"
#include "ins_j.h"
#include "ins_r.h"
#include "ins_s.h"
#include "ins_u.h"

int main(void) {
	printf("#include \"dynarec_x86_64.h\"\n\n");

#define X_R(MNEMONIC)                                                         \
	codegen_start_ins(#MNEMONIC);                                         \
	for (size_t i = 0; i < 8; i++) {                                      \
		codegen_##MNEMONIC((i & 1) >> 0, (i & 2) >> 1, (i & 4) >> 2); \
	}                                                                     \
	codegen_end_ins();
#define X_I(MNEMONIC)                                           \
	codegen_start_ins(#MNEMONIC);                           \
	for (size_t i = 0; i < 4; i++) {                        \
		codegen_##MNEMONIC((i & 1) >> 0, (i & 2) >> 1); \
	}                                                       \
	codegen_end_ins();
#define T(...) __VA_ARGS__
#define X_I_IMM(MNEMONIC, F12S, F7S)                                          \
	{                                                                     \
		codegen_start_ins(#MNEMONIC);                                 \
		int64_t f12s[] = {F12S}, f7s[] = {F7S};                       \
		for (size_t i = 0; i < sizeof(f12s) / sizeof(f12s[0]); i++) { \
			codegen_##MNEMONIC##_f12(f12s[i]);                    \
		}                                                             \
		for (size_t i = 0; i < sizeof(f7s) / sizeof(f7s[0]); i++) {   \
			codegen_##MNEMONIC##_f7(f7s[i]);                      \
		}                                                             \
		codegen_end_ins();                                            \
	}
#define X_S(MNEMONIC)                                           \
	codegen_start_ins(#MNEMONIC);                           \
	for (size_t i = 0; i < 4; i++) {                        \
		codegen_##MNEMONIC((i & 1) >> 0, (i & 2) >> 1); \
	}                                                       \
	codegen_end_ins();
#define X_B(MNEMONIC)                                           \
	codegen_start_ins(#MNEMONIC);                           \
	for (size_t i = 0; i < 4; i++) {                        \
		codegen_##MNEMONIC((i & 1) >> 0, (i & 2) >> 1); \
	}                                                       \
	codegen_end_ins();
#define X_U(MNEMONIC)                             \
	codegen_start_ins(#MNEMONIC);             \
	for (size_t i = 0; i < 2; i++) {          \
		codegen_##MNEMONIC((i & 1) >> 0); \
	}                                         \
	codegen_end_ins();
#define X_J(MNEMONIC)                             \
	codegen_start_ins(#MNEMONIC);             \
	for (size_t i = 0; i < 2; i++) {          \
		codegen_##MNEMONIC((i & 1) >> 0); \
	}                                         \
	codegen_end_ins();

	X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef T
#undef X_I_IMM
#undef X_S
#undef X_B
#undef X_U
#undef X_J

	return 0;
}

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>

#include "isa.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#include "ins_b.h"
#include "ins_i.h"
#include "ins_j.h"
#include "ins_r.h"
#include "ins_s.h"
#include "ins_u.h"

#pragma GCC diagnostic pop

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
#undef X_S
#undef X_B
#undef X_U
#undef X_J

	return 0;
}

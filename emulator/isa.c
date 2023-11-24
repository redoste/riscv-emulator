#include <stdint.h>

#include "isa.h"

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Woverride-init"

const ins_type_t INS_TYPES[0x20] = {
#define X_R(MNEMONIC, OPCODE, F3, F7, EXPR) [OPCODE >> 2] = INS_TYPE_R,
#define X_I(MNEMONIC, OPCODE, F3, EXPR)     [OPCODE >> 2] = INS_TYPE_I,
#define X_S(MNEMONIC, OPCODE, F3, EXPR)     [OPCODE >> 2] = INS_TYPE_S,
#define X_B(MNEMONIC, OPCODE, F3, EXPR)     [OPCODE >> 2] = INS_TYPE_B,
#define X_U(MNEMONIC, OPCODE, EXPR)         [OPCODE >> 2] = INS_TYPE_U,
#define X_J(MNEMONIC, OPCODE, EXPR)         [OPCODE >> 2] = INS_TYPE_J,

	X_INSTRUCTIONS

#undef X_R
#undef X_I
#undef X_S
#undef X_B
#undef X_J
};

#pragma GCC diagnostic pop

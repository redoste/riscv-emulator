#ifndef ISA_H
#define ISA_H

#include <rv64_isa.h>

/* X_INSTRUCTIONS : X-macro of all the instructions the codegen will emit x86-64
 *                  code for
 *     X_R(MNEMONIC) : R-type instruction
 *     X_I(MNEMONIC) : I-type instruction
 *     X_S(MNEMONIC) : S-type instruction
 *     X_B(MNEMONIC) : B-type instruction
 *     X_U(MNEMONIC) : U-type instruction
 *     X_J(MNEMONIC) : J-type instruction
 */
#define X_INSTRUCTIONS \
	X_R(ADD)       \
	X_R(SUB)       \
	X_R(SLL)       \
	X_R(SLT)       \
	X_R(SLTU)      \
	X_R(XOR)       \
	X_R(SRL)       \
	X_R(SRA)       \
	X_R(OR)        \
	X_R(AND)       \
                       \
	X_R(ADDW)      \
	X_R(SUBW)      \
	X_R(SLLW)      \
	X_R(SRLW)      \
	X_R(SRAW)      \
                       \
	X_R(MUL)       \
	X_R(MULH)      \
	X_R(MULHSU)    \
	X_R(MULHU)     \
	X_R(DIV)       \
	X_R(DIVU)      \
	X_R(REM)       \
	X_R(REMU)      \
                       \
	X_R(MULW)      \
	X_R(DIVW)      \
	X_R(DIVUW)     \
	X_R(REMW)      \
	X_R(REMUW)     \
                       \
	X_I(LB)        \
	X_I(LH)        \
	X_I(LW)        \
	X_I(LD)        \
	X_I(LBU)       \
	X_I(LHU)       \
	X_I(LWU)       \
                       \
	X_I(FENCE)     \
	X_I(FENCEI)    \
                       \
	X_I(ADDI)      \
	X_I(SLLI)      \
	X_I(SLTI)      \
	X_I(SLTIU)     \
	X_I(XORI)      \
	X_I(SRLI)      \
	X_I(ORI)       \
	X_I(ANDI)      \
                       \
	X_I(ADDIW)     \
	X_I(SLLIW)     \
	X_I(SRLIW)     \
                       \
	X_I(JALR)      \
                       \
	X_I(ECALL)     \
                       \
	X_I(CSRRW)     \
	X_I(CSRRS)     \
	X_I(CSRRC)     \
	X_I(CSRRWI)    \
	X_I(CSRRSI)    \
	X_I(CSRRCI)    \
                       \
	X_S(SB)        \
	X_S(SH)        \
	X_S(SW)        \
	X_S(SD)        \
                       \
	X_B(BEQ)       \
	X_B(BNE)       \
	X_B(BLT)       \
	X_B(BGE)       \
	X_B(BLTU)      \
	X_B(BGEU)      \
                       \
	X_U(AUIPC)     \
	X_U(LUI)       \
                       \
	X_J(JAL)

#endif

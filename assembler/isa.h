#ifndef ISA_H
#define ISA_H

#include <stdint.h>

#include <rv64_isa.h>

/* X_INSTRUCTIONS : X-macro storing informations about all the instructions
 *                  the assembler can assemble
 *     X_R(MNEMONIC, OPCODE, FUNCT3, FUNCT7) : R-type instruction
 *     X_R_A(MNEMONIC, OPCODE, FUNCT7, RS2)  : R-type instruction that is parsed like an instruction from the A
 *                                             extension
 *     X_I(MNEMONIC, OPCODE, FUNCT3)         : I-type instruction
 *     X_I_S(MNEMONIC, OPCODE, FUNCT3)       : I-type instruction that is parsed like a S-type (e.g. loads)
 *     X_S(MNEMONIC, OPCODE, FUNCT3)         : S-type instruction
 *     X_B(MNEMONIC, OPCODE, FUNCT3)         : B-type instruction
 *     X_U(MNEMONIC, OPCODE)                 : U-type instruction
 *     X_J(MNEMONIC, OPCODE)                 : J-type instruction
 *     X_P(MNEMONIC)                         : pseudo instruction or instructions that require specific parsing
 *                                             or encoding
 */
#define X_INSTRUCTIONS                               \
	X_R(ADD, OPCODE_OP, F3_ADD, F7_ADD)          \
	X_R(SUB, OPCODE_OP, F3_SUB, F7_SUB)          \
	X_R(SLL, OPCODE_OP, F3_SLL, F7_SLL)          \
	X_R(SLT, OPCODE_OP, F3_SLT, F7_SLT)          \
	X_R(SLTU, OPCODE_OP, F3_SLTU, F7_SLTU)       \
	X_R(XOR, OPCODE_OP, F3_XOR, F7_XOR)          \
	X_R(SRL, OPCODE_OP, F3_SRL, F7_SRL)          \
	X_R(SRA, OPCODE_OP, F3_SRA, F7_SRA)          \
	X_R(OR, OPCODE_OP, F3_OR, F7_OR)             \
	X_R(AND, OPCODE_OP, F3_AND, F7_AND)          \
                                                     \
	X_R(ADDW, OPCODE_OP_32, F3_ADD, F7_ADD)      \
	X_R(SUBW, OPCODE_OP_32, F3_SUB, F7_SUB)      \
	X_R(SLLW, OPCODE_OP_32, F3_SLL, F7_SLL)      \
	X_R(SRLW, OPCODE_OP_32, F3_SRL, F7_SRL)      \
	X_R(SRAW, OPCODE_OP_32, F3_SRA, F7_SRA)      \
                                                     \
	X_R(MUL, OPCODE_OP, F3_MUL, F7_MUL)          \
	X_R(MULH, OPCODE_OP, F3_MULH, F7_MULH)       \
	X_R(MULHSU, OPCODE_OP, F3_MULHSU, F7_MULHSU) \
	X_R(MULHU, OPCODE_OP, F3_MULHU, F7_MULHU)    \
	X_R(DIV, OPCODE_OP, F3_DIV, F7_DIV)          \
	X_R(DIVU, OPCODE_OP, F3_DIVU, F7_DIVU)       \
	X_R(REM, OPCODE_OP, F3_REM, F7_REM)          \
	X_R(REMU, OPCODE_OP, F3_REMU, F7_REMU)       \
                                                     \
	X_R(MULW, OPCODE_OP_32, F3_MUL, F7_MUL)      \
	X_R(DIVW, OPCODE_OP_32, F3_DIV, F7_DIV)      \
	X_R(DIVUW, OPCODE_OP_32, F3_DIVU, F7_DIVU)   \
	X_R(REMW, OPCODE_OP_32, F3_REM, F7_REM)      \
	X_R(REMUW, OPCODE_OP_32, F3_REMU, F7_REMU)   \
                                                     \
	X_R_A(LR, OPCODE_AMO, F7_LR, false)          \
	X_R_A(SC, OPCODE_AMO, F7_SC, true)           \
	X_R_A(AMOSWAP, OPCODE_AMO, F7_AMOSWAP, true) \
	X_R_A(AMOADD, OPCODE_AMO, F7_AMOADD, true)   \
	X_R_A(AMOXOR, OPCODE_AMO, F7_AMOXOR, true)   \
	X_R_A(AMOAND, OPCODE_AMO, F7_AMOAND, true)   \
	X_R_A(AMOOR, OPCODE_AMO, F7_AMOOR, true)     \
	X_R_A(AMOMIN, OPCODE_AMO, F7_AMOMIN, true)   \
	X_R_A(AMOMAX, OPCODE_AMO, F7_AMOMAX, true)   \
	X_R_A(AMOMINU, OPCODE_AMO, F7_AMOMINU, true) \
	X_R_A(AMOMAXU, OPCODE_AMO, F7_AMOMAXU, true) \
                                                     \
	X_I_S(LB, OPCODE_LOAD, F3_LB)                \
	X_I_S(LH, OPCODE_LOAD, F3_LH)                \
	X_I_S(LW, OPCODE_LOAD, F3_LW)                \
	X_I_S(LD, OPCODE_LOAD, F3_LD)                \
	X_I_S(LBU, OPCODE_LOAD, F3_LBU)              \
	X_I_S(LHU, OPCODE_LOAD, F3_LHU)              \
	X_I_S(LWU, OPCODE_LOAD, F3_LWU)              \
                                                     \
	X_I(ADDI, OPCODE_OP_IMM, F3_ADD)             \
	X_I(SLLI, OPCODE_OP_IMM, F3_SLL)             \
	X_I(SLTI, OPCODE_OP_IMM, F3_SLT)             \
	X_I(SLTIU, OPCODE_OP_IMM, F3_SLTU)           \
	X_I(XORI, OPCODE_OP_IMM, F3_XOR)             \
	X_I(SRLI, OPCODE_OP_IMM, F3_SRL)             \
	X_I(ORI, OPCODE_OP_IMM, F3_OR)               \
	X_I(ANDI, OPCODE_OP_IMM, F3_AND)             \
                                                     \
	X_I(ADDIW, OPCODE_OP_IMM_32, F3_ADD)         \
	X_I(SLLIW, OPCODE_OP_IMM_32, F3_SLL)         \
	X_I(SRLIW, OPCODE_OP_IMM_32, F3_SRL)         \
                                                     \
	X_I_S(JALR, OPCODE_JALR, F3_JALR)            \
                                                     \
	X_S(SB, OPCODE_STORE, F3_SB)                 \
	X_S(SH, OPCODE_STORE, F3_SH)                 \
	X_S(SW, OPCODE_STORE, F3_SW)                 \
	X_S(SD, OPCODE_STORE, F3_SD)                 \
	X_B(BEQ, OPCODE_BRANCH, F3_BEQ)              \
	X_B(BNE, OPCODE_BRANCH, F3_BNE)              \
	X_B(BLT, OPCODE_BRANCH, F3_BLT)              \
	X_B(BGE, OPCODE_BRANCH, F3_BGE)              \
	X_B(BLTU, OPCODE_BRANCH, F3_BLTU)            \
	X_B(BGEU, OPCODE_BRANCH, F3_BGEU)            \
                                                     \
	X_U(AUIPC, OPCODE_AUIPC)                     \
	X_U(LUI, OPCODE_LUI)                         \
                                                     \
	X_J(JAL, OPCODE_JAL)                         \
                                                     \
	X_P(J)                                       \
	X_P(LI)                                      \
	X_P(MV)                                      \
                                                     \
	X_P(SRAI)                                    \
	X_P(SRAIW)                                   \
                                                     \
	X_P(ECALL)                                   \
	X_P(EBREAK)                                  \
                                                     \
	X_P(FENCE)

/* ins_mnemonic_t : enumeration of instruction mnemonics that the assembler
 *                  can parse
 */
typedef enum ins_mnemonic_t {
#define X_R(MNEMONIC, O, F3, F7)    INS_##MNEMONIC,
#define X_R_A(MNEMONIC, O, F7, RS2) INS_##MNEMONIC,
#define X_I(MNEMONIC, O, F3)        INS_##MNEMONIC,
#define X_I_S(MNEMONIC, O, F3)      INS_##MNEMONIC,
#define X_S(MNEMONIC, O, F3)        INS_##MNEMONIC,
#define X_B(MNEMONIC, O, F3)        INS_##MNEMONIC,
#define X_U(MNEMONIC, O)            INS_##MNEMONIC,
#define X_J(MNEMONIC, O)            INS_##MNEMONIC,
#define X_P(MNEMONIC)               INS_##MNEMONIC,
	X_INSTRUCTIONS
#undef X_R
#undef X_R_A
#undef X_I
#undef X_I_S
#undef X_S
#undef X_B
#undef X_U
#undef X_J
#undef X_P

		INS_COUNT,
} ins_mnemonic_t;

/* INS_NAMES : string representation of the instruction mnemonics
 */
extern const char* const INS_NAMES[];

/* ins_attrib_t : enumeration of instruction attributes that the assembler
 *                can parse
 */
typedef enum ins_attrib_t {
	/* Attributes to the A extension instructions */
	INS_ATTRIB_AMO_AQ,
	INS_ATTRIB_AMO_RL,
	INS_ATTRIB_AMO_AQRL,

	/* Size attributes */
	INS_ATTRIB_SIZE_W,
	INS_ATTRIB_SIZE_D,

	INS_ATTRIB_COUNT,
} ins_attrib_t;

/* INS_ATTRIB_NAMES : string representation of the instruction attributes
 */
extern const char* const INS_ATTRIB_NAMES[];

/* REG_ALIAS : string representation of the register aliases
 */
extern const char* const REG_ALIAS[];

#endif

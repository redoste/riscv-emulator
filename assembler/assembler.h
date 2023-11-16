#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdbool.h>
#include <stdint.h>

#include "isa.h"
#include "lexer.h"

/* assembler_assemble_instruction : assemble a single instruction
 *                                  returns true if the instruction was assembled successfully
 *                                  returns false if assembling the instruction generated an error
 *                                  in case of error, the generated instruction should be considered invalid
 *     ins_mnemonic_t mnemonic : mnemonic of the instruction to assemble
 *     lexer_t* lexer          : lexer used to lex the operands of the instruction
 *     uint32_t* instruction   : pointer to the uint32_t where the assembled instruction will be written
 */
bool assembler_assemble_instruction(ins_mnemonic_t mnemonic, lexer_t* lexer, uint32_t* instruction);

#endif

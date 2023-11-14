#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stdbool.h>
#include <stdint.h>

#include "isa.h"
#include "lexer.h"

bool assembler_assemble_instruction(ins_mnemonic_t mnemonic, lexer_t* lexer, uint32_t* instruction);

#endif

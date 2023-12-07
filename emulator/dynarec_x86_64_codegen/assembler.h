#ifndef ASSEMBLER_H
#define ASSEMBLER_H

#include <stddef.h>

#include "x86_isa.h"

/* asm_x86_ins : assemble and encode a single x86-64 instruction
 *               returns the number of bytes used by the instruction
 *     uint8_t* out            : pointer to the buffer where the instruction will be written
 *     size_t out_size         : size of the buffer
 *     size_t* reloc_pos       : pointer to the size_t where the index of the relocation
 *                               will be written
 *     x86_mnemonic_t mnemonic : mnemonic of the instruction to assemble
 *     x86_operand_t dst       : destination operand
 *     x86_operand_t src       : source operand
 */
size_t asm_x86_ins(uint8_t* out, size_t out_size, size_t* reloc_pos, x86_mnemonic_t mnemonic, x86_operand_t dst, x86_operand_t src);

#endif

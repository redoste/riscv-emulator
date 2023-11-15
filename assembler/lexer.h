#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "isa.h"

// Capacity of the buffer used for reading a single mnemonic or operand
#define LEXER_WORD_BUFFER_CAP 32

// (Un)define to enable lexer debug logging
#define DEBUG_LEXER

typedef struct pos_t {
	const char* filename;
	size_t line;
	size_t col;
} pos_t;

#define POS_T_FMT_STR    "%s:%zu:%zu"
#define POS_T_FMT_ARG(p) (p).filename, (p).line, (p).col

typedef struct lexer_t {
	FILE* input_stream;
	pos_t pos;

	/* NOTE : for now the peek buffer is only one char deep
	 *        we can make it an array if required later
	 */
	char peek_buffer;
	bool peek_buffer_valid;
} lexer_t;

typedef enum token_type_t {
	TT_INS_MNEMONIC,
	TT_REG_OPERAND,
	TT_REG_DEREF_OPERAND,
	TT_INT_LITERAL,
	TT_COMMA,
	TT_EOI,
	TT_EOF,

	TT_COUNT,
} token_type_t;

typedef struct token_t {
	token_type_t type;
	union {
		ins_mnemonic_t as_ins_mnemonic;
		reg_t as_reg_operand;
		reg_t as_reg_deref_operand;
		int64_t as_int_literal;
	};
	pos_t pos;
} token_t;

void lexer_create(lexer_t* lexer, FILE* input_stream, const char* filename);
bool lexer_next(lexer_t* lexer, token_t* token);
bool lexer_next_expected(lexer_t* lexer, token_t* token, token_type_t expected_type, bool or_eox);

#ifdef DEBUG_LEXER
void lexer_debug_print_token(const token_t* token);
#endif

#endif

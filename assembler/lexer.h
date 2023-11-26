#ifndef LEXER_H
#define LEXER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "isa.h"

/* LEXER_WORD_BUFFER_CAP : Capacity of the buffer used for reading a single
 *                         mnemonic, operand or literal
 *                         32 characters are enough to store all the possible
 *                         mnemonics, operands or an integer literal up to 1 << 120
 */
#define LEXER_WORD_BUFFER_CAP 32

/* DEBUG_LEXER : (Un)define it to {disable,enable} lexer debug logging
 */
#define DEBUG_LEXER

/* pos_t : struct storing a position in a source file
 *         it's used both by the lexer to know its position and the tokens to
 *         report diagnostics with a user-friendly location
 */
typedef struct pos_t {
	const char* filename;
	size_t line;
	size_t col;
} pos_t;

/* POS_T_FMT_STR & POS_T_FMT_ARG : macros used to easily print a pos_t position in a
 *                                 format string
 */
#define POS_T_FMT_STR    "%s:%zu:%zu"
#define POS_T_FMT_ARG(p) (p).filename, (p).line, (p).col

/* lexer_t : struct storing the current state of the lexer
 *           the lexer have a singe input stream using libc stream features (FILE*)
 *           and a single byte "peek buffer" used to lookahead a single char in the
 *           stream
 */
typedef struct lexer_t {
	FILE* input_stream;
	pos_t pos;

	/* NOTE : for now the peek buffer is only one char deep
	 *        we can make it an array if it's required by more complex token types later
	 */
	char peek_buffer;
	bool peek_buffer_valid;
} lexer_t;

/* token_type_t : enumeration of the kinds of tokens
 */
typedef enum token_type_t {
	TT_INS_MNEMONIC,       // instruction mnemonic (e.g. `ld` `mv` `addi`)
	TT_REG_OPERAND,        // register operand (e.g. `x12` `zero` `a0`)
	TT_REG_DEREF_OPERAND,  // register dereference operand (e.g. `(a0)` `(sp)`)
	TT_INT_LITERAL,        // integer literal (e.g. `1337` `0x80` `-0xdeadc0de` `-1234`)
	TT_FENCE_OPERAND,      // fence operand (e.g. `iorw` `rw` `io`)
	TT_COMMA,              // comma : `,`
	TT_EOI,                // end of instruction : `\n` or `;`
	TT_EOF,                // end of file

	TT_COUNT,
} token_type_t;

/* token_t : struct representing a single token
 */
typedef struct token_t {
	token_type_t type;
	union {
		ins_mnemonic_t as_ins_mnemonic;
		reg_t as_reg_operand;
		reg_t as_reg_deref_operand;
		int64_t as_int_literal;
		uint8_t as_fence_operand;
	};
	pos_t pos;
} token_t;

/* lexer_create : create a lexer
 *     lexer_t* lexer       : pointer to the lexer_t struct to initialize
 *     FILE* input_stream   : input stream
 *     const char* filename : input stream filename (only used for diagnostics)
 */
void lexer_create(lexer_t* lexer, FILE* input_stream, const char* filename);

/* lexer_next : lex the next token in the input stream
 *              returns true if the new token is valid
 *              returns false if lexing the next token generated an error
 *              in case of error, the generated token_t struct should be considered *invalid*
 *     lexer_t* lexer : pointer to the lexer
 *     token_t* token : pointer to the token struct to fill with the new token
 */
bool lexer_next(lexer_t* lexer, token_t* token);

/* lexer_next_expected : lex the next token in the input_stream but accept only a specific kind of tokens
 *                       returns true if the new token is valid and of the correct kind
 *                       returns false if lexing the next token generated an error or if the new token is
 *                       of an unexpected kind
 *                       in case of error, the generated token_t struct should be considered *invalid*
 *     lexer_t* lexer             : pointer to the lexer
 *     token_t* token             : pointer to the token struct to fill with the new token
 *     token_type_t expected_type : expected kind for the next token
 *     bool or_eox                : also accept TT_EOI and TT_EOL tokens
 */
bool lexer_next_expected(lexer_t* lexer, token_t* token, token_type_t expected_type, bool or_eox);

#ifdef DEBUG_LEXER
/* lexer_debug_print_token : print to stderr details about a token
 *                           (used for debugging)
 *     const token_t* token : token to print
 */
void lexer_debug_print_token(const token_t* token);
#endif

#endif

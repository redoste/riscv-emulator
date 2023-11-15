#include <assert.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "isa.h"
#include "lexer.h"

void lexer_create(lexer_t* lexer, FILE* input_stream) {
	lexer->input_stream = input_stream;
	lexer->line = 1;
	lexer->col = 1;
	lexer->peek_buffer_valid = false;
}

static int lexer_peek_char(lexer_t* lexer) {
	if (lexer->peek_buffer_valid) {
		return lexer->peek_buffer;
	}

	int c = fgetc(lexer->input_stream);
	if (c > 0) {
		lexer->peek_buffer = c;
		lexer->peek_buffer_valid = true;
		return c;
	} else {
		if (!feof(lexer->input_stream)) {
			perror("fgetc");
		}
		return -1;
	}
}

static int lexer_read_char(lexer_t* lexer) {
	if (lexer->peek_buffer_valid) {
		lexer->col++;
		lexer->peek_buffer_valid = false;
		return lexer->peek_buffer;
	}

	int c = fgetc(lexer->input_stream);
	if (c > 0) {
		lexer->col++;
		return c;
	} else {
		if (!feof(lexer->input_stream)) {
			perror("fgetc");
		}
		return -1;
	}
}

static void lexer_consume_peek(lexer_t* lexer) {
	if (!lexer->peek_buffer_valid) {
		fprintf(stderr, "ICE : Peek buffer isn't supposed to be empty\n");
		abort();
	}
	lexer->col++;
	lexer->peek_buffer_valid = false;
}

static void lexer_read_word(lexer_t* lexer, char* word, size_t word_capacity) {
	int c;
	size_t word_len = 0;
	do {
		c = lexer_peek_char(lexer);
		if (c > 0 && isalnum(c)) {
			lexer_consume_peek(lexer);
			word[word_len++] = c;
		}
	} while (c > 0 && word_len < word_capacity && isalnum(c));
}

static int lexer_parse_register(const char* word) {
	if (word[0] == 'x' && word[1] != 0) {
		char* reg_endptr;
		unsigned long long reg = strtoull(&word[1], &reg_endptr, 10);
		if ((size_t)(reg_endptr - word) != strlen(word) || reg >= REG_COUNT) {
			return -1;
		}
		return reg;
	}

	for (size_t reg = 0; reg < REG_COUNT; reg++) {
		if (strcasecmp(word, REG_ALIAS[reg]) == 0) {
			return reg;
		}
	}

	return -1;
}

static bool lexer_next_integer(lexer_t* lexer, token_t* token) {
	char minus = lexer_peek_char(lexer);
	bool negative = minus == '-';
	if (negative) {
		lexer_consume_peek(lexer);
	}

	char word[LEXER_WORD_BUFFER_CAP] = {0};
	lexer_read_word(lexer, word, LEXER_WORD_BUFFER_CAP - 1);

	int base = 10;
	size_t prefix = 0;
	if (strncasecmp(word, "0x", 2) == 0) {
		base = 16;
		prefix = 2;
	}

	char* endptr;
	// TODO : better overflow detection
	uint64_t integer = strtoull(&word[prefix], &endptr, base);
	if ((size_t)(endptr - word) != strlen(word)) {
		fprintf(stderr, "Invalid integer literal '%s' at %zu:%zu\n",
			word, token->pos.line, token->pos.col);
		return false;
	}

	token->type = TT_INT_LITERAL;
	if (negative) {
		// TODO : detect underflow
		token->as_signed_int_literal = -integer;
	} else {
		token->as_unsigned_int_literal = integer;
	}
	return true;
}

static bool lexer_next_reg_deref(lexer_t* lexer, token_t* token) {
	assert(lexer_peek_char(lexer) == '(');
	lexer_consume_peek(lexer);
	char word[LEXER_WORD_BUFFER_CAP] = {0};
	lexer_read_word(lexer, word, LEXER_WORD_BUFFER_CAP - 1);
	int reg = lexer_parse_register(word);
	if (reg < 0) {
		fprintf(stderr, "Invalid register dereference '%s' at %zu:%zu\n",
			word, token->pos.line, token->pos.col);
		return false;
	}

	int c = lexer_peek_char(lexer);
	if (c != ')') {
		fprintf(stderr, "Expected ')' at %zu:%zu\n",
			lexer->line, lexer->col);
		return false;
	}
	lexer_consume_peek(lexer);

	token->type = TT_REG_DEREF_OPERAND;
	token->as_reg_deref_operand = reg;
	return true;
}

static bool lexer_next_word(lexer_t* lexer, token_t* token) {
	char word[LEXER_WORD_BUFFER_CAP] = {0};
	lexer_read_word(lexer, word, LEXER_WORD_BUFFER_CAP - 1);

	for (size_t ins = 0; ins < INS_COUNT; ins++) {
		if (strcasecmp(word, INS_NAMES[ins]) == 0) {
			token->type = TT_INS_MNEMONIC;
			token->as_ins_mnemonic = ins;
			return true;
		}
	}

	int reg = lexer_parse_register(word);
	if (reg >= 0) {
		token->type = TT_REG_OPERAND;
		token->as_reg_operand = reg;
		return true;
	} else {
		fprintf(stderr, "Unexpected word '%s' at %zu:%zu\n", word, token->pos.line, token->pos.col);
		return false;
	}
}

bool lexer_next(lexer_t* lexer, token_t* token) {
	while (true) {
		token->pos.col = lexer->col;
		token->pos.line = lexer->line;

		int c = lexer_peek_char(lexer);
		if (c < 0) {
			if (feof(lexer->input_stream)) {
				token->type = TT_EOF;
				return true;
			} else {
				return false;
			}
		}

		if (c == '#') {
			// Comment
			do {
				c = lexer_read_char(lexer);
			} while (c > 0 && c != '\n');
			lexer->line++;
			lexer->col = 1;
			token->type = TT_EOI;
			return true;
		} else if (c == '\n') {
			// New line
			lexer_consume_peek(lexer);
			lexer->line++;
			lexer->col = 1;
			token->type = TT_EOI;
			return true;
		} else if (c == ';') {
			// Semicolon
			lexer_consume_peek(lexer);
			token->type = TT_EOI;
			return true;
		} else if (c == ',') {
			// Comma
			lexer_consume_peek(lexer);
			token->type = TT_COMMA;
			return true;
		} else if (isspace(c)) {
			// White space : nop
			lexer_consume_peek(lexer);
		} else if (isdigit(c) || c == '-') {
			// Integer literal
			return lexer_next_integer(lexer, token);
		} else if (c == '(') {
			// Register deref in address operand
			return lexer_next_reg_deref(lexer, token);
		} else if (isalpha(c)) {
			// Instruction mnemonic or register operand
			return lexer_next_word(lexer, token);
		} else {
			// TODO : better diag infrastructure (l:c at the start)
			fprintf(stderr, "Unexpected character '%c' at %zu:%zu\n",
				c, lexer->line, lexer->col);
			return false;
		}
	}
}

static const char* const TT_NAMES[TT_COUNT] = {
	[TT_INS_MNEMONIC] = "instruction mnemonic",
	[TT_REG_OPERAND] = "register operand",
	[TT_REG_DEREF_OPERAND] = "register dereference operand",
	[TT_INT_LITERAL] = "integer literal",
	[TT_COMMA] = "comma",
	[TT_EOI] = "end of instruction",
	[TT_EOF] = "end of file",
};

bool lexer_next_expected(lexer_t* lexer, token_t* token, token_type_t expected_type, bool or_eox) {
	bool ret = lexer_next(lexer, token);
#ifdef DEBUG_LEXER
	if (ret) {
		lexer_debug_print_token(token);
	}
#endif

	if (!ret) {
		return false;
	} else if (token->type == expected_type ||
		   (or_eox && token->type == TT_EOF) ||
		   (or_eox && token->type == TT_EOI)) {
		return true;
	} else {
		fprintf(stderr, or_eox ? "Expected %s, EOF or EOI but got %s at %zu:%zu\n" : "Expected %s but got %s at %zu:%zu\n",
			TT_NAMES[expected_type], TT_NAMES[token->type],
			token->pos.line, token->pos.col);
		return false;
	}
}

#ifdef DEBUG_LEXER
void lexer_debug_print_token(const token_t* token) {
	switch (token->type) {
		case TT_INS_MNEMONIC:
			fprintf(stderr, "%zu:%zu INS MNEMONIC %s\n", token->pos.line, token->pos.col,
				INS_NAMES[token->as_ins_mnemonic]);
			break;
		case TT_REG_OPERAND:
			fprintf(stderr, "%zu:%zu REG OPERAND x%d\n", token->pos.line, token->pos.col,
				token->as_reg_operand);
			break;
		case TT_REG_DEREF_OPERAND:
			fprintf(stderr, "%zu:%zu REG DEREF OPERAND (x%d)\n", token->pos.line, token->pos.col,
				token->as_reg_deref_operand);
			break;
		case TT_INT_LITERAL:
			fprintf(stderr, "%zu:%zu INT LITERAL 0x%016lx\n", token->pos.line, token->pos.col,
				token->as_unsigned_int_literal);
			break;
		case TT_COMMA:
			fprintf(stderr, "%zu:%zu COMMA\n", token->pos.line, token->pos.col);
			break;
		case TT_EOI:
			fprintf(stderr, "%zu:%zu EOI\n", token->pos.line, token->pos.col);
			break;
		case TT_EOF:
			fprintf(stderr, "%zu:%zu EOF\n", token->pos.line, token->pos.col);
			break;
		default:
			fprintf(stderr, "ICE : Invalid token type\n");
			abort();
			break;
	}
}
#endif

#include <stdarg.h>
#include <stdio.h>

#include "diag.h"
#include "lexer.h"

__attribute__((format(printf, 2, 3))) void diag_error(pos_t pos, const char* fmtstr, ...) {
	va_list ap;

	va_start(ap, fmtstr);
	fprintf(stderr, POS_T_FMT_STR ": error: ", POS_T_FMT_ARG(pos));
	vfprintf(stderr, fmtstr, ap);
	va_end(ap);
}

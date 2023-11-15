#ifndef DIAG_H
#define DIAG_H

#include "lexer.h"

__attribute__((format(printf, 2, 3))) void diag_error(pos_t pos, const char* fmtstr, ...);

#endif

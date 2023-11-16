#ifndef DIAG_H
#define DIAG_H

#include "lexer.h"

/* diag_error : report an error diagnostic to stderr
 *     pos_t pos          : position of the error in the input stream
 *     const char* fmtstr : format string of the error
 *     ...                : format string arguments
 */
__attribute__((format(printf, 2, 3))) void diag_error(pos_t pos, const char* fmtstr, ...);

#endif

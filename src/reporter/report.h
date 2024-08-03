#include <lexer/lexer.h>
#include <parser/parser.h>
#include <pool/token.h>

#ifndef REPORTER_H
#define REPORTER_H

void report_error(token_t *token, struct Lexer *lexer, const char *message);

#endif
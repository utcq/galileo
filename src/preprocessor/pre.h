#include <pool/token.h>
#ifndef PREPROCESSOR_H
#define PREPROCESSOR_H

void pre_process_macrodef(struct Lexer *lexer);
void pre_process_macroref(struct Lexer *lexer, token_t *token, struct macro_stream *macro);

#endif
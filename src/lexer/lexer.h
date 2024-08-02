#include <stdint.h>
#include <stdio.h>
#include <pool/token.h>

#ifndef LEXER_H
#define LEXER_H

#define MAX_INT_LEN 20
#define MAX_IDENT_LEN 100

struct token_stream {
  token_t *token;
  struct token_stream *next;
};

struct Lexer { 
  char *source;
  uint32_t pos;
  char p_tok;
  uint32_t len;
  struct token_stream *last_token;
  struct token_stream *tokens_begin;
};

struct Lexer *lexer_create(char *source, size_t len);
void lexer_parse(struct Lexer *lexer);

int lexer_utils_isalpha(char c);
int lexer_utils_isnum(char c);
int lexer_utils_iskeyword(char *str);
int lexer_utils_isprefix(struct Lexer *lexer);
void lexer_append_token(struct Lexer *lexer, token_t *token);
void __lexer__dump_tokens(struct Lexer *lexer);

#endif
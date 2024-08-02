#include <pool/token.h>
#include <lexer/lexer.h>
#include <string.h>
#include <stdlib.h>

#define ALLOC_TOKEN(name) token_t *name = (token_t *)malloc(sizeof(token_t))


struct Lexer *lexer_create(char *source, size_t len) {
  struct Lexer *lexer = (struct Lexer *)malloc(sizeof(struct Lexer));
  lexer->source =source;
  lexer->pos=0;
  lexer->p_tok = source[0];
  lexer->len=len;
  lexer->last_token=NULL;
  lexer->tokens_begin=NULL;
  return lexer;
}


void lexer_advance(struct Lexer *lexer, int ns) {
  lexer->pos += ns;
  if (lexer->pos < lexer->len) {
    lexer->p_tok = lexer->source[lexer->pos];
  }
}

char lexer_peak(struct Lexer *lexer, int ns) {
  return lexer->source[lexer->pos + ns];
}

char lexer_eat(struct Lexer *lexer) {
  char c = lexer->p_tok;
  lexer_advance(lexer, 1);
  return c;
}

void lexer_parse_int_literal(struct Lexer *lexer) {
  char *literal = (char*)malloc(MAX_INT_LEN);
  ALLOC_TOKEN(token);
  token->pos = lexer->pos;
  token->value = literal;
  token->type = TOKEN_TYPE_INTEGER;

  for (char c = lexer_eat(lexer); lexer_utils_isnum(c); c = lexer_eat(lexer)) {
    strncat(literal, &c, 1);
  }

  return lexer_append_token(lexer, token);
}

void lexer_parse_multi_ps(struct Lexer *lexer) {
  char *literal = (char*)malloc(MAX_INT_LEN);
  ALLOC_TOKEN(token);
  token->pos = lexer->pos;
  token->value = literal;
  token->type = TOKEN_TYPE_IDENTIFIER;

  for (char c = lexer_eat(lexer); lexer_utils_isalpha(c); c = lexer_eat(lexer)) {
    strncat(literal, &c, 1);
  }

  if (lexer_utils_iskeyword(literal)) {
    token->type = TOKEN_TYPE_KEYWORD;
  }

  return lexer_append_token(lexer, token);
}

void lexer_discriminator(struct Lexer *lexer) {
  if (lexer_utils_isnum(lexer->p_tok)) {
    lexer_parse_int_literal(lexer);
  }
  else if (lexer_utils_isalpha(lexer->p_tok)) {
    lexer_parse_multi_ps(lexer);
  }
}

void lexer_parse(struct Lexer *lexer) {
  while (lexer->pos < lexer->len) {
    lexer_discriminator(lexer);
  }
}
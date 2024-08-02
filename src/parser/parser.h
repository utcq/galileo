#include <pool/token.h>
#ifndef PARSER_H
#define PARSER_H

struct Parser {
  struct Lexer *lexer;
  struct token_stream *current_token;
};

void parser_advance(struct Parser *parser, int ns);
token_t *parser_eat(struct Parser *parser);
token_t *parser_peek(struct Parser *parser, int ns);

struct Parser *parser_create(struct Lexer *lexer);
void parser_parse(struct Parser *parser);

#endif
#include <pool/token.h>
#include <ast/scope.h>

#ifndef PARSER_H
#define PARSER_H

struct Parser {
  struct Lexer *lexer;
  struct token_stream *current_token;
  struct pt_scope *global_scope;
  char **specifiers;
  unsigned char spec_count;
};

void parser_advance(struct Parser *parser, int ns);
token_t *parser_eat(struct Parser *parser);
token_t *parser_peek(struct Parser *parser, int ns);

void parser_discriminator(struct Parser *parser, struct pt_scope *scope);
struct Parser *parser_create(struct Lexer *lexer);
void parser_parse(struct Parser *parser);

void __parser_dump_scope(struct Parser *parser);

#endif
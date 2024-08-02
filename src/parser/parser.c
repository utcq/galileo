#include <stdlib.h>
#include <lexer/lexer.h>
#include <pool/log.h>
#include <pool/token.h>
#include <parser/parser.h>
#include <string.h>

void parser_advance(struct Parser *parser, int ns) {
  for (int i = 0; i < ns; i++) {
    if (parser->current_token->next) {
      parser->current_token = parser->current_token->next;
    } else {
      parser->current_token = NULL;
      return;
    }
  }
}

token_t *parser_eat(struct Parser *parser) {
  token_t *token = parser->current_token->token;
  parser_advance(parser, 1);
  return token;
}

token_t *parser_peek(struct Parser *parser, int ns) {
  struct token_stream *current = parser->current_token;
  for (int i = 0; i < ns; i++) {
    if (current->next) {
      current = current->next;
    } else { return NULL; }
  }
  return current->token;
}

token_t *parser_askfor(struct Parser *parser, token_type_t type, const char *value) {
  token_t *token = parser_eat(parser);
  if (token->type != type || (value && strcmp(token->value, value) != 0)) {
    ERROR("Unexpected token %s", token->value);
    return NULL;
  }
  return token;
}

void parser_parse_function(struct Parser *parser) {
  parser_advance(parser, 1); // Skip the prefix
  token_t *type = parser_askfor(parser, TOKEN_TYPE_IDENTIFIER, NULL);
  token_t *name = parser_askfor(parser, TOKEN_TYPE_IDENTIFIER, NULL);
  if (!type || !name) { return; }

  parser_askfor(parser, TOKEN_TYPE_LPAREN, NULL);
  while (parser->current_token->token->type != TOKEN_TYPE_RPAREN) {
    token_t *arg = parser_askfor(parser, TOKEN_TYPE_IDENTIFIER, NULL);
    parser_askfor(parser, TOKEN_TYPE_SEMICOLON, NULL);
    token_t *arg_type = parser_askfor(parser, TOKEN_TYPE_IDENTIFIER, NULL);
    if (!arg || !arg_type) { return; }

    if (parser_peek(parser, 1)->type == TOKEN_TYPE_COMMA) {
      parser_advance(parser, 1);
    }
  }
  parser_askfor(parser, TOKEN_TYPE_RPAREN, NULL);
  DEBUG("Parsed function %s %s\n", type->value, name->value);

}

void parser_discriminator(struct Parser *parser) {
  token_t *token = parser->current_token->token;
  if (token->type == TOKEN_TYPE_PREFIX && (strcmp(token->value, "f!") == 0)) {
    parser_parse_function(parser);
  } else {
    WARN("Unknown token %s\n", token->value);
    parser_advance(parser, 1);
  }
}

void parser_parse(struct Parser *parser) {
  while (parser->current_token) {
    parser_discriminator(parser);
  }
}

struct Parser *parser_create(struct Lexer *lexer) {
  struct Parser *parser = (struct Parser *)malloc(sizeof(struct Parser));
  if (!parser) {
    perror("Failed to allocate memory for parser");
    return NULL;
  }
  parser->lexer = lexer;
  parser->current_token = lexer->tokens_begin;
  return parser;
}
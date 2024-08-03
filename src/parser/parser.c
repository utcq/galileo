#include <stdlib.h>
#include <lexer/lexer.h>
#include <pool/log.h>
#include <pool/token.h>
#include <parser/parser.h>
#include <ast/scope.h>
#include <reporter/report.h>
#include <parser/exp.h>
#include <string.h>

#define AUTO_ASSERT(x) if (!x) { return; }

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
    report_error(token, parser->lexer, "Unexpected Token");
    return NULL;
  }
  return token;
}

void parser_parse_function(struct Parser *parser, struct pt_scope *scope) {
  struct function_declaration *fn_decl = malloc(sizeof(struct function_declaration));

  parser_advance(parser, 1); // Skip the prefix
  token_t *type = parser_askfor(parser, TOKEN_TYPE_IDENTIFIER, NULL);
  token_t *name = parser_askfor(parser, TOKEN_TYPE_IDENTIFIER, NULL);
  if (!type || !name) { return; }

  fn_decl->name = name->value;
  fn_decl->type = type->value;
  fn_decl->scope = scope_new_scope(scope, fn_decl->name);
  fn_decl->scope->type = SCOPE_FUNCTION;
  struct function_parameter *fn_params;

  AUTO_ASSERT(parser_askfor(parser, TOKEN_TYPE_LPAREN, NULL));
  while (parser->current_token->token->type != TOKEN_TYPE_RPAREN) {
    token_t *arg = parser_askfor(parser, TOKEN_TYPE_IDENTIFIER, NULL);
    parser_askfor(parser, TOKEN_TYPE_COLON, NULL);
    token_t *arg_type = parser_askfor(parser, TOKEN_TYPE_IDENTIFIER, NULL);
    if (!arg || !arg_type) { return; }

    if (parser_peek(parser, 1)->type == TOKEN_TYPE_COMMA) {
      parser_advance(parser, 1);
    }
    if (!fn_params) {
      fn_params = malloc(sizeof(struct function_parameter));
    } else {
      fn_params->next = malloc(sizeof(struct function_parameter));
      fn_params = fn_params->next;
    }
    fn_params->name = arg->value;
    fn_params->type = arg_type->value;
    struct variable_declaration *var_decl = malloc(sizeof(struct variable_declaration));
    var_decl->name = arg->value;
    var_decl->type = arg_type->value;
    struct declaration_v *arg_gen_decl = malloc(sizeof(struct declaration_v));
    arg_gen_decl->type = DECLARATION_VARIABLE;
    arg_gen_decl->data.var_decl = var_decl;
    scope_new_declaration(fn_decl->scope, var_decl->name, arg_gen_decl);
  }
  AUTO_ASSERT(parser_askfor(parser, TOKEN_TYPE_RPAREN, NULL));

  fn_decl->parameters = fn_params;  
  struct declaration_v *gen_decl = malloc(sizeof(struct declaration_v));
  gen_decl->type = DECLARATION_FUNCTION;
  gen_decl->data.fn_decl = fn_decl;
  scope_new_declaration(scope, name->value, gen_decl);
  
  AUTO_ASSERT(parser_askfor(parser, TOKEN_TYPE_LBRACE, NULL));
  while (parser->current_token->token->type != TOKEN_TYPE_RBRACE) {
    parser_discriminator(parser, fn_decl->scope);
  }
  AUTO_ASSERT(parser_askfor(parser, TOKEN_TYPE_RBRACE, NULL));
}


void parser_parse_return(struct Parser *parser, struct pt_scope *scope) {
  token_t *tester = parser_eat(parser);
  if (scope->type != SCOPE_FUNCTION) {
    report_error(tester, parser->lexer, "Return statement outside of function");
    return;
  }
  struct expression_node *exp = exp_parser_parse_semicolon(parser);
}

void parser_discriminator(struct Parser *parser, struct pt_scope *scope) {
  token_t *token = parser->current_token->token;
  if (token->type == TOKEN_TYPE_PREFIX && (strcmp(token->value, "f!") == 0)) {
    parser_parse_function(parser, scope);
  } 
  
  else if (token->type == TOKEN_TYPE_KEYWORD && (strcmp(token->value, "return") == 0)) {
    parser_parse_return(parser, scope);
  }
  
  else {
    WARN("Unknown token %s\n", token->value);
    parser_advance(parser, 1);
  }
}

void parser_parse(struct Parser *parser) {
  while (parser->current_token) {
    parser_discriminator(parser, parser->global_scope);
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
  parser->global_scope = scope_new_scope(NULL, NULL);
  return parser;
}

void __scope_dump(struct pt_scope *scope);

void __dump_func(struct declaration_map_child *child) {
  struct function_declaration *fn_decl = child->value->data.fn_decl;
  DEBUG("Function Key: [%s]\n", child->key);
  DEBUG("Function Name: [%s]\n", fn_decl->name);
  DEBUG("Function Type: [%s]\n", fn_decl->type);
  DEBUG("Function Scope: [%p]\n", fn_decl->scope);
  DEBUG("Function Scope Name: [%s]\n", fn_decl->scope->scope_name);
  DEBUG("Function Scope Parent: [%p]\n", fn_decl->scope->parent);
  DEBUG("Function Scope Parent Name: [%s]\n", fn_decl->scope->parent->scope_name);
  struct function_parameter *fn_params = fn_decl->parameters;
  while (fn_params) {
    DEBUG("Function Parameter Name: [%s]\n", fn_params->name);
    DEBUG("Function Parameter Type: [%s]\n", fn_params->type);
    fn_params = fn_params->next;
  }
  struct pt_scope *fn_scope = fn_decl->scope;
  __scope_dump(fn_scope);
}

void __dump_variable(struct declaration_map_child *child) {
  struct variable_declaration *var_decl = child->value->data.var_decl;
  DEBUG("Variable Key: [%s]\n", child->key);
  DEBUG("Variable Name: [%s]\n", var_decl->name);
  DEBUG("Variable Type: [%s]\n", var_decl->type);
}

void __scope_dump(struct pt_scope *scope) {
  for (uint i = 0; i < scope->declarations->pos; i++) {
    struct declaration_map_child *child = scope->declarations->data[i];
    switch (child->value->type) {
      case DECLARATION_FUNCTION:
        __dump_func(child);
        break;
      case DECLARATION_VARIABLE:
        __dump_variable(child);
        break;
      default:
        WARN("Unknown declaration type %d\n", child->value->type);
        break;
    }
  }
}

void __parser_dump_scope(struct Parser *parser) {
  struct pt_scope *scope = parser->global_scope;
  __scope_dump(scope);
}
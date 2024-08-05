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

void parser_decl_addspec(struct Parser *parser, struct declaration_v *decl) {
  for (uint i = 0; i < parser->spec_count; i++) {
    decl->specifiers[i] = parser->specifiers[i];
  }
  parser->spec_count=0;
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
  struct function_parameter *fn_params = NULL;

  AUTO_ASSERT(parser_askfor(parser, TOKEN_TYPE_LPAREN, NULL));
  while (parser->current_token->token->type != TOKEN_TYPE_RPAREN) {
    token_t *arg = parser_askfor(parser, TOKEN_TYPE_IDENTIFIER, NULL);
    parser_askfor(parser, TOKEN_TYPE_COLON, NULL);
    token_t *arg_type = parser_askfor(parser, TOKEN_TYPE_IDENTIFIER, NULL);
    if (!arg || !arg_type) { return; }

    if (parser->current_token->token->type == TOKEN_TYPE_COMMA) {
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
    arg_gen_decl->specifiers = NULL; // Parameters can't have specifiers
    scope_new_declaration(fn_decl->scope, var_decl->name, arg_gen_decl);
  }
  AUTO_ASSERT(parser_askfor(parser, TOKEN_TYPE_RPAREN, NULL));

  fn_decl->parameters = fn_params;
  struct declaration_v *gen_decl = malloc(sizeof(struct declaration_v));
  gen_decl->type = DECLARATION_FUNCTION;
  gen_decl->data.fn_decl = fn_decl;
  gen_decl->specifiers = (char**)malloc(sizeof(char*) * MAX_SPECIFIERS);
  memset(gen_decl->specifiers, 0, sizeof(char*)*MAX_SPECIFIERS);
  parser_decl_addspec(parser, gen_decl);
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
  struct statement_node *stmt = malloc(sizeof(struct statement_node));
  stmt->actual = malloc(sizeof(struct statement_actual));
  stmt->actual->type = STATEMENT_RETURN;
  stmt->actual->data.return_.expression = exp;
  stmt->next = NULL;
  scope_add_statement(scope, stmt);

}

void parser_parse_spec(struct Parser *parser) {
  if (parser->spec_count >= MAX_SPECIFIERS) {
      report_error(parser->current_token->token, parser->lexer, "Too many specifiers");
      return;
  }
  token_t *token = parser_eat(parser);
  parser->specifiers[parser->spec_count++] = token->value;
}

void parser_parse_variable(struct Parser *parser, struct pt_scope *scope) {
  parser_advance(parser, 1); // Skip keyword
  token_t *name = parser_askfor(parser, TOKEN_TYPE_IDENTIFIER, NULL);
  parser_askfor(parser, TOKEN_TYPE_COLON, NULL);
  token_t *type = parser_askfor(parser, TOKEN_TYPE_IDENTIFIER, NULL);
  if (!name || !type) { return; }

  parser_askfor(parser, TOKEN_TYPE_ASSIGN, NULL);
  struct expression_node *exp = exp_parser_parse_semicolon(parser);
  struct variable_declaration *var_decl = malloc(sizeof(struct variable_declaration));
  var_decl->name = name->value;
  var_decl->type = type->value;
  var_decl->value = exp;
  struct declaration_v *gen_decl = malloc(sizeof(struct declaration_v));
  gen_decl->type = DECLARATION_VARIABLE;
  gen_decl->data.var_decl = var_decl;
  gen_decl->specifiers = (char**)malloc(sizeof(char*) * MAX_SPECIFIERS);
  memset(gen_decl->specifiers, 0, sizeof(char*)*MAX_SPECIFIERS);
  parser_decl_addspec(parser, gen_decl);
  struct statement_node *stmt = malloc(sizeof(struct statement_node));
  stmt->actual = malloc(sizeof(struct statement_actual));
  stmt->actual->type = STATEMENT_DECLARATION;
  stmt->actual->data.declaration.declaration = gen_decl;
  stmt->next = NULL;
  scope_add_statement(scope, stmt);
}


void parser_parse_exp_stat(struct Parser *parser) {
  struct expression_node *exp = exp_parser_parse_semicolon(parser);
  struct statement_node *stmt = malloc(sizeof(struct statement_node));
  stmt->actual = malloc(sizeof(struct statement_actual));
  stmt->actual->type = STATEMENT_EXPRESSION;
  stmt->actual->data.expression.expst = exp;
  stmt->next = NULL;
  scope_add_statement(parser->global_scope, stmt);
}


void parser_discriminator(struct Parser *parser, struct pt_scope *scope) {
  token_t *token = parser->current_token->token;
  if (token->type == TOKEN_TYPE_PREFIX && (strcmp(token->value, "f!") == 0)) {
    parser_parse_function(parser, scope);
  } 
  
  else if (token->type == TOKEN_TYPE_KEYWORD && (strcmp(token->value, "return") == 0)) {
    parser_parse_return(parser, scope);
  }

  else if (token->type == TOKEN_TYPE_DECORATOR) {
    parser_parse_spec(parser);
    return;
  }

  else if (token->type == TOKEN_TYPE_KEYWORD && (strcmp(token->value, "var") == 0)) {
    parser_parse_variable(parser, scope);
  }

  else {
    parser_parse_exp_stat(parser);
    //WARN("Unexpected token %s\n", token->value);
    //parser_advance(parser, 1);
  }
  parser->spec_count=0;
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
  parser->spec_count=0;
  parser->specifiers = (char**)malloc(sizeof(char*) * MAX_SPECIFIERS);
  memset(parser->specifiers, 0, sizeof(char*)*MAX_SPECIFIERS);
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
  PRINT("\n");
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

void __dump_literal(struct expression_node *expr) {
  switch (expr->data.literal.value_type) {
    case TOKEN_TYPE_INTEGER:
      DEBUG("Literal Type: [Integer] %d\n", expr->data.literal.value.int_value);
      break;
    case TOKEN_TYPE_STRING:
      DEBUG("Literal Type: [String] %s\n", expr->data.literal.value.str_value);
      break;
    default:
      WARN("Unknown literal type %d\n", expr->data.literal.value_type);
      break;
  }
}

void __dump_expr(struct expression_node *expr) {
  switch (expr->type) {
    case EXPR_LITERAL:
      __dump_literal(expr);
      break;
    case EXPR_VARIABLE:
      DEBUG("Expression Type: [Variable] %s\n", expr->data.variable.name);
      break;
    case EXPR_BINARY_OP:
      DEBUG("Expression Type: [Binary Op]\n");
      __dump_expr(expr->data.binary_op.left);
      DEBUG("Operator: [%d]\n", expr->data.binary_op.op);
      __dump_expr(expr->data.binary_op.right);
      break;
    default:
      WARN("Unknown expression type %d\n", expr->type);
      break;
  }
}

void __dump_var_decl(struct variable_declaration *var_decl) {
  DEBUG("Variable Name: [%s]\n", var_decl->name);
  DEBUG("Variable Type: [%s]\n", var_decl->type);
  if (var_decl->value) {
    DEBUG("Variable Value: \n");
    __dump_expr(var_decl->value);
  }
}

void __dump_declaration(struct statement_node *stmt) {
  if (stmt->actual->data.declaration.declaration->specifiers) {
    for (int i=0; stmt->actual->data.declaration.declaration->specifiers[i] != NULL; i++) {
      DEBUG("Specifier: [%s]\n", stmt->actual->data.declaration.declaration->specifiers[i]);
    }
  }
  switch (stmt->actual->data.declaration.declaration->type) {
    case DECLARATION_VARIABLE:
      DEBUG("Declaration Type: [Variable]\n");
      __dump_var_decl(stmt->actual->data.declaration.declaration->data.var_decl);
      break;
    default:
      WARN("Unknown declaration type %d\n", stmt->actual->data.declaration.declaration->type);
      break;
  }
}

void __dump_stat(struct statement_node *stmt) {
  switch (stmt->actual->type) {
    case STATEMENT_RETURN:
      DEBUG("Statement Type: [Return]\n");
      __dump_expr(stmt->actual->data.return_.expression);
      break;
    case STATEMENT_DECLARATION:
      DEBUG("Statement Type: [Declaration]\n");
      __dump_declaration(stmt);
      break;
    default:
      WARN("Unknown statement type %d\n", stmt->actual->type);
      break;
  }
  PRINT("\n");
}

void __dump_decl(struct declaration_map_child *child) {
  if (child->value->specifiers) {
    for (int i=0; child->value->specifiers[i] != NULL; i++) {
      DEBUG("Specifier: [%s]\n", child->value->specifiers[i]);
    }
  }

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

void __scope_dump(struct pt_scope *scope) {
  for (uint i = 0; i < scope->declarations->pos; i++) {
    struct declaration_map_child *child = scope->declarations->data[i];
    __dump_decl(child);
  }
  
  PRINT("\n");

  struct statement_node *stmt = scope->statements;
  while (stmt) {
    __dump_stat(stmt);
    stmt = stmt->next;
  }
  PRINT("\n");
}

void __parser_dump_scope(struct Parser *parser) {
  struct pt_scope *scope = parser->global_scope;
  __scope_dump(scope);
}
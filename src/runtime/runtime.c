#include <FFI/ffi.h>
#include <stdlib.h>
#include <runtime/rt.h>
#include <ast/scope.h>
#include <reporter/report.h>
#include <pool/log.h>
#include <string.h>

void *ffi_lib;

void runtime_declare_variable(struct pt_scope *scope, struct declaration_v *decl) {
  struct declaration_v *rt_decl = (struct declaration_v *)malloc(sizeof(struct declaration_v));
  rt_decl->type = DECLARATION_VARIABLE;
  rt_decl->data.var_decl = (struct variable_declaration *)malloc(sizeof(struct variable_declaration));
  rt_decl->data.var_decl->name = decl->data.var_decl->name;
  rt_decl->data.var_decl->type = decl->data.var_decl->type;
  rt_decl->data.var_decl->value = runtime_eval_expression(scope, decl->data.var_decl->value);
  scope_new_declaration(
    scope, decl->data.var_decl->name,
    rt_decl
  );
}

void runtime_declare(struct pt_scope *scope, struct declaration_v *decl) {
  switch (decl->type) {
    case DECLARATION_VARIABLE:
      // only supported decl rn
      runtime_declare_variable(scope, decl);
      break;
    
    default:
      ERROR("Unknown declaration type %d\n", decl->type);
      exit(EXIT_FAILURE);
      break;
  }
}

struct declaration_map_child *runtime_resolve_decl(struct pt_scope *scope, char *key) {
  struct declaration_map_child *decl = scope_get_declaration(scope, key);
  if (!decl) {
    if (scope->parent) {
      return runtime_resolve_decl(scope->parent, key);
    }
    ERROR("Declaration not found\n");
    exit(EXIT_FAILURE);
    return NULL;
  }
  return decl;
}

struct expression_node *runtime_fn_call(struct pt_scope *scope, struct expression_node *exp) {
  struct declaration_map_child *decl = runtime_resolve_decl(scope, exp->data.function_call.name);

  if (decl->value->type != DECLARATION_FUNCTION) {
    report_error(NULL, NULL, "Reference is not a function");
    return NULL;
  }

  struct pt_scope *fn_scope = decl->value->data.fn_decl->scope;

  int is_extern=0;
  for (int i=0; decl->value->specifiers[i] != NULL; i++) {
    if (strcmp(decl->value->specifiers[i], "extern") == 0) {
      is_extern=1;
    }
  }

  int64_t *ffi_args = NULL;
  if (is_extern) {
    ffi_args = (int64_t *)malloc(sizeof(int64_t)*exp->data.function_call.argument_count);
  }
  
  int i=0;
  struct function_parameter *arg = decl->value->data.fn_decl->parameters;
  while (arg) {
    if (!exp->data.function_call.arguments) {
      ERROR("Missing arguments\n");
      exit(EXIT_FAILURE);
      return NULL;
    }
    if (
      (!exp->data.function_call.arguments[i])
      //TODO: || (exp->data.function_call.arguments[i]->type != arg->type)
    ) {
      ERROR("Missing argument/s\n");
      exit(EXIT_FAILURE);
      return NULL;
    }
    
    if (is_extern) {
      ffi_args[i] = (int64_t)(
        runtime_eval_expression(scope, exp->data.function_call.arguments[i])->data
      ).literal.value.str_value; // Str Value as it is 8 bytes
    } else {

      scope_get_declaration(fn_scope, arg->name)->value->data.var_decl->value = runtime_eval_expression(scope,
        exp->data.function_call.arguments[i]
      ); // set arg in scope to be processed expression

    }
    arg = arg->next; i++;
  }

  if (is_extern) {
    DEBUG("Executing FFI %s\n", decl->key);
    FFI_translation_level(ffi_lib, decl, exp->data.function_call.argument_count, ffi_args);
    return NULL;
  }

  return runtime_exec_stats(fn_scope);
}

struct expression_node *__int2expr(int val) {
  struct expression_node *exp = (struct expression_node *)malloc(sizeof(struct expression_node));
  exp->type = EXPR_LITERAL;
  exp->data.literal.value_type = TOKEN_TYPE_INTEGER;
  exp->data.literal.value.int_value = val;
  return exp;
}

struct expression_node *runtime_eval_int_int_binop(int left, int right, operator_t op) {
  switch (op) {
    case OP_ADD:
      return __int2expr(left + right);
    
    case OP_SUBTRACT:
      return __int2expr(left - right);
    
    case OP_MULTIPLY:
      return __int2expr(left * right);
    
    case OP_DIVIDE:
      return __int2expr(left / right);

    default:
      ERROR("Unknown operator %d\n", op);
      exit(EXIT_FAILURE);
      break;
  }
}

struct expression_node *runtime_eval_lit_lit_binop(struct expression_node *left, struct expression_node *right, operator_t op) {

  if (left->data.literal.value_type == TOKEN_TYPE_INTEGER && right->data.literal.value_type == TOKEN_TYPE_INTEGER) {
    return runtime_eval_int_int_binop(
      left->data.literal.value.int_value, right->data.literal.value.int_value,
      op
    );
  }

  ERROR("Unsupported operation\n");
  exit(EXIT_FAILURE);
  return NULL;
}

struct expression_node *runtime_eval_binary_op(struct pt_scope *scope, struct expression_node *exp) {
  struct expression_node *left = runtime_eval_expression(scope, exp->data.binary_op.left);
  struct expression_node *right = runtime_eval_expression(scope, exp->data.binary_op.right);
  if (left->type == EXPR_LITERAL && right->type == EXPR_LITERAL) {
    return runtime_eval_lit_lit_binop(left, right, exp->data.binary_op.op);
  }

  DEBUG("Unsupported operation types %d %d\n", left->type, right->type);

  ERROR("Unsupported operation types\n");
  exit(EXIT_FAILURE);
  return NULL;
}

struct expression_node *runtime_resolve_var(struct pt_scope *scope, struct expression_node *exp) {
  struct variable_declaration *decl = runtime_resolve_decl(scope, exp->data.variable.name)->value->data.var_decl;
  return decl->value;
}

struct expression_node *runtime_eval_expression(struct pt_scope *scope, struct expression_node *exp) {
  switch (exp->type) {
    
    case EXPR_FUNCTION_CALL:
      return runtime_fn_call(scope, exp);
    
    case EXPR_LITERAL:
      return exp;
    
    case EXPR_VARIABLE:
      return runtime_resolve_var(scope, exp);
    
    case EXPR_BINARY_OP:
      return runtime_eval_binary_op(scope, exp);

    default:
      ERROR("Unknown expression type %d\n", exp->type);
      exit(EXIT_FAILURE);
      return NULL;
  }
}

struct expression_node *runtime_run_stat(struct pt_scope *scope, struct statement_actual *actual) {
  switch (actual->type) {

    case STATEMENT_DECLARATION:
      runtime_declare(scope, actual->data.declaration.declaration);
      break;

    case STATEMENT_EXPRESSION:
      runtime_eval_expression(scope, actual->data.expression.expst);
      break;
    
    case STATEMENT_RETURN:
      return runtime_eval_expression(scope, actual->data.return_.expression);
    
    default:
      ERROR("Unknown statement type %d\n", actual->type);
      exit(EXIT_FAILURE);
      break;
  }
  return NULL;
}

struct expression_node *runtime_exec_stats(struct pt_scope *scope) {
  DEBUG("Executing scope %s\n", scope->scope_name);

  struct statement_node *current_stmt = scope->statements;
  struct expression_node *ret = NULL;
  while (current_stmt) {
    ret = runtime_run_stat(scope, current_stmt->actual);
    current_stmt = current_stmt->next;
  }

  if (!ret) {
    ret = (struct expression_node *)malloc(sizeof(struct expression_node));
    ret->type = EXPR_NULL;
    ret->data.null.null_val = NULL;
  }
  return ret;
}

void runtime_execute(struct pt_scope *scope) {
  ffi_lib = FFI_load_lib("./stdlib.so");
  runtime_exec_stats(scope); // exec gloabal scope
  /*struct declaration_map_child *decl = scope_get_declaration(scope, "tst_var");
  if (decl) {
    DEBUG("%s = %d\n", decl->value->data.var_decl->name, decl->value->data.var_decl->value->data.literal.value.int_value);
  }*/
  /*struct expression_node *val = scope_get_declaration(
    scope_get_declaration(scope, "main")->value->data.fn_decl->scope,
    "r"
  )->value->data.var_decl->value;
  DEBUG("main.r = %d\n", val->data.literal.value.int_value);*/
}
#include <stdlib.h>
#include <runtime/rt.h>
#include <ast/scope.h>
#include <reporter/report.h>
#include <pool/log.h>

void runtime_declare_variable(struct pt_scope *scope, struct declaration_v *decl) {
  scope_new_declaration(scope, decl->data.var_decl->name, decl);
}

void runtime_declare(struct pt_scope *scope, struct declaration_v *decl) {
  switch (decl->type) {
    case DECLARATION_VARIABLE:
      // only supported decl rn
      runtime_declare_variable(scope, decl);
      break;
    
    default:
      ERROR("Unknown declaration type %d\n", decl->type);
      break;
  }
}

struct expression_node *runtime_fn_call(struct pt_scope *scope, struct expression_node *exp) {
  struct declaration_map_child *decl = scope_get_declaration(scope, exp->data.function_call.name);
  if (!decl) {
    report_error(NULL, NULL, "Function not found");
    return NULL;
  }

  if (decl->value->type != DECLARATION_FUNCTION) {
    report_error(NULL, NULL, "Reference is not a function");
    return NULL;
  }

  struct function_parameter *arg = decl->value->data.fn_decl->parameters;
  int i=0;
  while (arg) {
    


    arg = arg->next; i++;
  }
}

struct expression_node *runtime_eval_expression(struct pt_scope *scope, struct expression_node *exp) {
  switch (exp->type) {
    
    case EXPR_FUNCTION_CALL:
      return runtime_fn_call(scope, exp);

    default:
      ERROR("Unknown expression type %d\n", exp->type);
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

    default:
      ERROR("Unknown statement type %d\n", actual->type);
      break;
  }
  return NULL;
}

struct expression_node *runtime_exec_stats(struct pt_scope *scope) {
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
  runtime_exec_stats(scope); // exec gloabal scope
  struct declaration_map_child *decl = scope_get_declaration(scope, "tst_var");
  if (decl) {
    DEBUG("%s = %d\n", decl->value->data.var_decl->name, decl->value->data.var_decl->value->data.literal.value.int_value);
  }
}
#ifndef RUNTIME_H
#define RUNTIME_H

#include <ast/scope.h>
#include <pool/token.h>

struct expression_node *runtime_eval_expression(struct pt_scope *scope, struct expression_node *exp);
struct expression_node *runtime_exec_stats(struct pt_scope *scope);
void runtime_execute(struct pt_scope *scope);

#endif
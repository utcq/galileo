#include <parser/parser.h>
#include <ast/scope.h>
#ifndef EXPRESSION_PARSER_H
#define EXPRESSION_PARSER_H

struct expression_node *exp_parser_parse_function_call(struct Parser *parser);
struct expression_node *exp_parser_parse_comma_or_rparen(struct Parser *parser);
struct expression_node *exp_parser_parse_semicolon(struct Parser *parser);
operator_t get_operator_type(const char *op);
int get_operator_precedence(operator_t op);
struct expression_node *exp_parser_parse_primary(struct Parser *parser);
struct expression_node *exp_parser_parse_expression(struct Parser *parser, int precedence);

#endif
#include <parser/parser.h>
#include <parser/exp.h>
#include <reporter/report.h>
#include <pool/log.h>
#include <pool/token.h>
#include <lexer/lexer.h>
#include <ast/scope.h>
#include <stdlib.h>
#include <string.h>

struct expression_node *create_expression_node(expr_type_t type) {
    struct expression_node *node = malloc(sizeof(struct expression_node));
    if (!node) {
        perror("Failed to allocate memory for expression node");
        exit(EXIT_FAILURE);
    }
    node->type = type;
    return node;
}

struct expression_node *exp_parser_parse_literal(struct Parser *parser) {
    struct expression_node *node = create_expression_node(EXPR_LITERAL);
    node->data.literal.value = atoi(parser->current_token->token->value);
    parser_advance(parser, 1);
    return node;
}

struct expression_node *exp_parser_parse_variable(struct Parser *parser) {
    struct expression_node *node = create_expression_node(EXPR_VARIABLE);
    node->data.variable.name = strdup(parser->current_token->token->value);
    parser_advance(parser, 1);
    return node;
}

struct expression_node *exp_parser_parse_binary_op(struct Parser *parser, struct expression_node *left, int precedence) {
    while (parser->current_token && parser->current_token->token->type == TOKEN_TYPE_OPERATOR) {
        operator_t op = get_operator_type(parser->current_token->token->value);
        int op_precedence = get_operator_precedence(op);

        if (op_precedence < precedence) {
            break;
        }

        parser_advance(parser, 1);

        struct expression_node *right = exp_parser_parse_primary(parser);

        if (parser->current_token && parser->current_token->token->type == TOKEN_TYPE_OPERATOR) {
            int next_precedence = get_operator_precedence(get_operator_type(parser->current_token->token->value));
            if (op_precedence < next_precedence) {
                right = exp_parser_parse_binary_op(parser, right, op_precedence + 1);
            }
        }

        struct expression_node *new_node = create_expression_node(EXPR_BINARY_OP);
        new_node->data.binary_op.op = op;
        new_node->data.binary_op.left = left;
        new_node->data.binary_op.right = right;
        left = new_node;
    }
    return left;
}

struct expression_node *exp_parser_parse_primary(struct Parser *parser) {
    if (parser->current_token->token->type == TOKEN_TYPE_INTEGER || parser->current_token->token->type == TOKEN_TYPE_STRING) {
        return exp_parser_parse_literal(parser);
    } else if (parser->current_token->token->type == TOKEN_TYPE_IDENTIFIER) {
        return exp_parser_parse_variable(parser);
    } else if (parser->current_token->token->type == TOKEN_TYPE_LPAREN) {
        parser_advance(parser, 1);
        struct expression_node *node = exp_parser_parse_expression(parser, 0);
        if (parser->current_token->token->type != TOKEN_TYPE_RPAREN) {
            report_error(parser->current_token->token, parser->lexer, "Expected closing parenthesis");
            return NULL;
        }
        parser_advance(parser, 1);
        return node;
    } else {
        report_error(parser->current_token->token, parser->lexer, "Unexpected token");
        return NULL;
    }
}

struct expression_node *exp_parser_parse_expression(struct Parser *parser, int precedence) {
    struct expression_node *left = exp_parser_parse_primary(parser);
    if (!left) {
        return NULL;
    }
    return exp_parser_parse_binary_op(parser, left, precedence);
}

struct expression_node *exp_parser_parse_semicolon(struct Parser *parser) {
    struct expression_node *root = exp_parser_parse_expression(parser, 0);
    if (!root) {
        return NULL;
    }
    if (parser->current_token->token->type != TOKEN_TYPE_SEMICOLON) {
        report_error(parser->current_token->token, parser->lexer, "Expected semicolon");
        return NULL;
    }
    parser_advance(parser, 1); // Skip Semicolon
    return root;
}

operator_t get_operator_type(const char *op) {
    if (strcmp(op, "+") == 0) return OP_ADD;
    if (strcmp(op, "-") == 0) return OP_SUBTRACT;
    if (strcmp(op, "*") == 0) return OP_MULTIPLY;
    if (strcmp(op, "/") == 0) return OP_DIVIDE;
    return -1;
}

int get_operator_precedence(operator_t op) {
    switch (op) {
        case OP_ADD:
        case OP_SUBTRACT:
            return 1;
        case OP_MULTIPLY:
        case OP_DIVIDE:
            return 2;
        // TODO: More operators
        default:
            return 0;
    }
}
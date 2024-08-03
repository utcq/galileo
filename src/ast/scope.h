#ifndef SCOPE_H
#define SCOPE_H

#include <stdint.h>
typedef enum {
  DECLARATION_VARIABLE,
  DECLARATION_FUNCTION,
  DECLARATION_STRUCT,
  DECLARATION_ENUM,
  DECLARATION_TYPEDEF
} declaration_type_t;

typedef enum {
  VALUE_TYPE_INT,
  VALUE_TYPE_STR,
  VALUE_TYPE_BOOL,
  VALUE_TYPE_FLOAT,
  VALUE_TYPE_CHAR,
  VALUE_TYPE_VOID,
  VALUE_TYPE_IDENTIFIER
} value_type_t;

typedef enum {
  SCOPE_GLOBAL,
  SCOPE_FUNCTION
} scope_type_t;

struct function_parameter {
  char *name;
  char *type;
  struct function_parameter *next;
};

struct function_declaration {
  char *name;
  char *type;
  struct function_parameter *parameters;
  struct pt_scope *scope;
};

struct variable_declaration {
  char *name;
  char *type;
};

struct declaration_v {
  declaration_type_t type;
  union {
    // ... (TODO: Types)
    struct function_declaration *fn_decl;
    struct variable_declaration *var_decl;
  } data;
};

/* Scope struct:
- Parent scope ptr, NULL if global scope
- Scope name, NULL if global scope
- Exponentially growing children array
- Exponentially growing declaration hashmap (key: symbol name, value: declaration_v)

I want to build the tree from scratch
I would say:
  length = (length*2) if (i+1>length) else (length)
That will cause auto-filling regions in memory
*/

// Parse Time Scope

struct scope_children {
  uint32_t length;
  uint32_t pos;    // pos != length. length = max_capacity. pos = current_index
  struct pt_scope **data;
};

void scope_append_child(struct pt_scope *scope, struct pt_scope *child);

struct declaration_map_child {
  char *key;
  struct declaration_v *value;
};

struct declaration_hashmap {
  uint32_t length;
  uint32_t pos;
  struct declaration_map_child **data;
};

void scope_new_declaration(struct pt_scope *scope, char *key, struct declaration_v *value);
struct declaration_map_child *scope_get_declaration(struct pt_scope *scope, char *key);

struct pt_scope {
  scope_type_t type;
  struct pt_scope *parent;
  char *scope_name;
  struct scope_children children;
  struct declaration_hashmap *declarations;
};

struct pt_scope *scope_new_scope(struct pt_scope *parent, char *scope_name);

typedef enum {
    EXPR_LITERAL,
    EXPR_BINARY_OP,
    EXPR_VARIABLE
} expr_type_t;

typedef enum {
    OP_ADD,
    OP_SUBTRACT,
    OP_MULTIPLY,
    OP_DIVIDE
} operator_t;

struct expression_node {
    expr_type_t type;
    union {
        struct {
            int value;
        } literal;
        struct {
            operator_t op;
            struct expression_node *left;
            struct expression_node *right;
        } binary_op;
        struct {
            char *name;
        } variable;
    } data;
};

#endif
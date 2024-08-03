#include <ast/scope.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct pt_scope *scope_new_scope(struct pt_scope *parent, char *scope_name) {
  struct pt_scope *scope = (struct pt_scope*)malloc(sizeof(struct pt_scope));
  memset(scope, 0, sizeof(struct pt_scope));
  scope->parent = parent;
  scope->scope_name = scope_name;
  scope->declarations = (struct declaration_hashmap*)malloc(sizeof(struct declaration_hashmap));
  return scope;
}

struct declaration_map_child *scope_get_declaration(struct pt_scope *scope, char *key) {
  for (uint i=0; i < scope->declarations->pos;i++) {
    if (
      strcmp(scope->declarations->data[i]->key, key) == 0
    ) {
      return scope->declarations->data[i];
    }
  }
  return NULL;
}

void __scope_decl_memman(struct scope_children *arr /* Generic */) {
  if (arr->pos+1 >= arr->length) {
    arr->length = arr->length*2;
    arr->data = realloc(arr->data, arr->length);
  }
}

void scope_new_declaration(struct pt_scope *scope, char *key, struct declaration_v *value) {
  struct declaration_map_child *child = malloc(sizeof(struct declaration_map_child));
  child->key = key;
  child->value = value;  
  __scope_decl_memman((struct scope_children*)scope->declarations);
  scope->declarations->data[scope->declarations->pos++] = child;
}

void scope_append_child(struct pt_scope *scope, struct pt_scope *child) {
  __scope_decl_memman(&scope->children);
  scope->children.data[scope->children.pos++] = child;
}
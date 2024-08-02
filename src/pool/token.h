#include <stdint.h>

#ifndef TOKEN_H
#define TOKEN_H

typedef enum {
  TOKEN_TYPE_IDENTIFIER,
  TOKEN_TYPE_KEYWORD,
  TOKEN_TYPE_INTEGER,
  TOKEN_TYPE_STRING,
  TOKEN_TYPE_PREFIX,
  TOKEN_TYPE_SYMBOL
} token_type_t;

typedef struct {
  char *value;
  token_type_t type;
  uint32_t pos;
  uint32_t len;
} token_t;

#endif
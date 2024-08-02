#include <pool/token.h>
#include <lexer/lexer.h>
#include <preprocessor/pre.h>
#include <stdlib.h>
#include <string.h>


void pre_process_macrodef(struct Lexer *lexer, token_t *token) {
  struct macro_stream *macro = (struct macro_stream *)malloc(sizeof(struct macro_stream));
  macro->name = token->value;
  macro->value = (char *)malloc(lexer->len);
  macro->next = NULL;

  for (char c = lexer_eat(lexer); c != '\n'; c = lexer_eat(lexer)) {
    strncat(macro->value, &c, 1);
  }

  struct macro_stream *last = lexer->macros;
  if (last == NULL) {
    lexer->macros = macro;
  } else {
    while (last->next != NULL) {
      last = last->next;
    }
    last->next = macro;
  }
}
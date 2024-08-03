#include <pool/token.h>
#include <lexer/lexer.h>
#include <preprocessor/pre.h>
#include <stdlib.h>
#include <string.h>


void pre_process_macrodef(struct Lexer *lexer) {
  struct macro_stream *macro = (struct macro_stream *)malloc(sizeof(struct macro_stream));
  if (!macro) {
      perror("Failed to allocate memory for macro_stream");
      return;
  }
  macro->value = (char *)malloc(MAX_MACRO_LEN);
  if (!macro->value) {
      perror("Failed to allocate memory for macro value");
      free(macro);
      return;
  }
  macro->value[0] = '\0';
  macro->next = NULL;

  lexer_advance(lexer, 1);
  char *name = (char *)malloc(MAX_IDENT_LEN);
  if (!name) {
      perror("Failed to allocate memory for macro name");
      free(macro->value);
      free(macro);
      return;
  }
  name[0] = '\0';
  macro->name = name;

  for (char c = lexer_eat(lexer); c != ' ' && c != '\0'; c = lexer_eat(lexer)) {
      strncat(name, &c, 1);
  }
  macro->name = realloc(macro->name, strlen(macro->name) + 1);


  for (char c = lexer_eat(lexer); c != '\n' && c != '\0'; c = lexer_eat(lexer)) {
      if (
        (c == 92 && lexer_peek(lexer, 1) == 32)
      ) {
        lexer_eat(lexer);
        continue;
      }
      if (c == ' ' || c == '\t' || c == '\r') { continue; }
      strncat(macro->value, &c, 1);
  }
  macro->value = realloc(macro->value, strlen(macro->value) + 1);
  macro->value[strlen(macro->value)] = '\0';

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

void pre_process_macroref(struct Lexer *lexer, token_t *token, struct macro_stream *macro) {
  char *macro_val = (char*)malloc(strlen(macro->value) + 1);
  if (!macro_val) {
      perror("Failed to allocate memory for macro value");
      return;
  }
  strcpy(macro_val, macro->value);

  struct Lexer *macro_lexer = lexer_create(macro_val, strlen(macro->value));
  if (!macro_lexer) {
      perror("Failed to create lexer for macro value");
      return;
  }
  macro_lexer->macros = lexer->macros;

  lexer_parse(macro_lexer);
  struct token_stream *ts = macro_lexer->tokens_begin;
  while (ts != NULL) {
    ts->token->pos += token->pos;
    lexer_append_token(lexer, ts->token);
    ts = ts->next;
  }
  free(macro_val);

  lexer->pos = token->pos + token->len;
  free(macro_lexer);
}
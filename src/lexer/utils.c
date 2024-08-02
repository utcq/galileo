#include <pool/token.h>
#include <lexer/lexer.h>
#include <stdlib.h>
#include <string.h>

const char *KEYWORDS[] = {
  "pub"
};


int lexer_utils_isalpha(char c) {
  if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '.') {
    return 1;
  }
  return 0;
}


int lexer_utils_isnum(char c) {
  if (c >= '0' && c <= '9') {
    return 1;
  }
  return 0;
}

int lexer_utils_iskeyword(char *str) {
  for (int i = 0; i < sizeof(KEYWORDS) / sizeof(KEYWORDS[0]); i++) {
    if (strcmp(KEYWORDS[i], str) == 0) {
      return 1;
    }
  }
  return 0;
}

void lexer_append_token(struct Lexer *lexer, token_t *token) {
  struct token_stream *ts = (struct token_stream *)malloc(sizeof(struct token_stream));
  ts->token = token;
  ts->next = NULL;
  if (!lexer->last_token) {
    lexer->tokens_begin = ts;
  } else {
    lexer->last_token->next = ts;
  }
  lexer->last_token = ts;
}
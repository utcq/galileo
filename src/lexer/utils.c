#include <pool/token.h>
#include <lexer/lexer.h>
#include <stdlib.h>
#include <string.h>

const char *KEYWORDS[] = {
  "pub"
};

const char *PREFIXES[] = {
  "f!",
  "m!def"
};

struct symbol_entry SYMBOLS[] = {
  {"(", TOKEN_TYPE_LPAREN},
  {")", TOKEN_TYPE_RPAREN},
  {"{", TOKEN_TYPE_LBRACE},
  {"}", TOKEN_TYPE_RBRACE},
  {";", TOKEN_TYPE_SEMICOLON},
  {"[", TOKEN_TYPE_LBRACKET},
  {"]", TOKEN_TYPE_RBRACKET},
  {",", TOKEN_TYPE_COMMA},
  {"#", TOKEN_TYPE_HASH},
  {"/", TOKEN_TYPE_SLASH},
  {"++", TOKEN_TYPE_INC},
  {"--", TOKEN_TYPE_DEC},
  {"==", TOKEN_TYPE_EQ},
  {"!=", TOKEN_TYPE_NEQ},
  {"<=", TOKEN_TYPE_LTE},
  {">=", TOKEN_TYPE_GTE},
  {"&&", TOKEN_TYPE_AND},
  {"||", TOKEN_TYPE_OR},
  {"<<", TOKEN_TYPE_LSHIFT},
  {">>", TOKEN_TYPE_RSHIFT},
  {"*", TOKEN_TYPE_ASTERISK},
  {"+", TOKEN_TYPE_PLUS},
  {"-", TOKEN_TYPE_MINUS},
  {"=", TOKEN_TYPE_ASSIGN},
  {"!", TOKEN_TYPE_BANG},
  {"<", TOKEN_TYPE_LT},
  {">", TOKEN_TYPE_GT},
  {"&", TOKEN_TYPE_AMP},
  {"|", TOKEN_TYPE_PIPE},
  {"^", TOKEN_TYPE_CARET},
  {"~", TOKEN_TYPE_TILD},
  {"%", TOKEN_TYPE_PERCENT},
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
  for (unsigned i = 0; i < sizeof(KEYWORDS) / sizeof(KEYWORDS[0]); i++) {
    if (strcmp(KEYWORDS[i], str) == 0) {
      return 1;
    }
  }
  return 0;
}

int lexer_utils_issymbol(struct Lexer *lexer) {
  for (unsigned i = 0; i < sizeof(SYMBOLS) / sizeof(SYMBOLS[0]); i++) {
    if (strncmp(lexer->source + lexer->pos, SYMBOLS[i].value, strlen(SYMBOLS[i].value)) == 0) {
      return strlen(SYMBOLS[i].value);
    }
  }
  return 0;
}

struct macro_stream *lexer_utils_ismacro(struct Lexer *lexer, char *name) {
  struct macro_stream *macro = lexer->macros;
  while (macro != NULL) {
    if (strcmp(macro->name, name) == 0) {
      return macro;
    }
    macro = macro->next;
  }
  return NULL;
}

struct symbol_entry *lexer_utils_get_symbol(struct Lexer *lexer) {
  for (unsigned i = 0; i < sizeof(SYMBOLS) / sizeof(SYMBOLS[0]); i++) {
    if (strncmp(lexer->source + lexer->pos, SYMBOLS[i].value, strlen(SYMBOLS[i].value)) == 0) {
      return &SYMBOLS[i];
    }
  }
  return NULL;
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

int lexer_utils_isprefix(struct Lexer *lexer) {
  for (unsigned i=0; i  < sizeof(PREFIXES) / sizeof(PREFIXES[0]); i++) {
    if (strncmp(lexer->source + lexer->pos, PREFIXES[i], strlen(PREFIXES[i])) == 0) {
      return strlen(PREFIXES[i]);
    }
  }
  return 0;
}
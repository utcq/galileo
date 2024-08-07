#include <pool/token.h>
#include <lexer/lexer.h>
#include <stdlib.h>
#include <string.h>

const char *DECORATOR[] = {
  "pub",
  "const",
  "static",
  "extern"
};

const char *KEYWORDS[] = {
  "return",
  "var",
  "tl_loadlib"
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
  {":", TOKEN_TYPE_COLON},
  {"[", TOKEN_TYPE_LBRACKET},
  {"]", TOKEN_TYPE_RBRACKET},
  {",", TOKEN_TYPE_COMMA},
  {"#", TOKEN_TYPE_COMMA},
  {"/", TOKEN_TYPE_OPERATOR},
  {"++", TOKEN_TYPE_OPERATOR},
  {"--", TOKEN_TYPE_OPERATOR},
  {"==", TOKEN_TYPE_OPERATOR},
  {"!=", TOKEN_TYPE_OPERATOR},
  {"<=", TOKEN_TYPE_OPERATOR},
  {">=", TOKEN_TYPE_OPERATOR},
  {"&&", TOKEN_TYPE_OPERATOR},
  {"||", TOKEN_TYPE_OPERATOR},
  {"<<", TOKEN_TYPE_OPERATOR},
  {">>", TOKEN_TYPE_OPERATOR},
  {"*", TOKEN_TYPE_OPERATOR},
  {"+", TOKEN_TYPE_OPERATOR},
  {"-", TOKEN_TYPE_OPERATOR},
  {"=", TOKEN_TYPE_ASSIGN},
  {"!", TOKEN_TYPE_OPERATOR},
  {"<", TOKEN_TYPE_OPERATOR},
  {">", TOKEN_TYPE_OPERATOR},
  {"&", TOKEN_TYPE_OPERATOR},
  {"|", TOKEN_TYPE_OPERATOR},
  {"^", TOKEN_TYPE_OPERATOR},
  {"~", TOKEN_TYPE_OPERATOR},
  {"%", TOKEN_TYPE_OPERATOR},
  {"$", TOKEN_TYPE_OPERATOR}
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

int lexer_utils_isdecorator(char *str) {
  for (unsigned i = 0; i < sizeof(DECORATOR) / sizeof(DECORATOR[0]); i++) {
    if (strcmp(DECORATOR[i], str) == 0) {
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
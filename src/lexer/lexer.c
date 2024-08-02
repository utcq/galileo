#include <pool/token.h>
#include <pool/log.h>
#include <lexer/lexer.h>
#include <string.h>
#include <stdlib.h>
#include <preprocessor/pre.h>

#define ALLOC_TOKEN(name) token_t *name = (token_t *)malloc(sizeof(token_t))
#define CLEANUP_SIZE(token) token->value = (char*)realloc(token->value, token->len + 1)

extern const char *PREFIXES[];

struct Lexer *lexer_create(char *source, size_t len) {
  struct Lexer *lexer = (struct Lexer *)malloc(sizeof(struct Lexer));
  lexer->source =source;
  lexer->pos=0;
  lexer->p_tok = source[0];
  lexer->len=len;
  lexer->last_token=NULL;
  lexer->tokens_begin=NULL;
  lexer->macros=NULL;
  return lexer;
}


void lexer_advance(struct Lexer *lexer, int ns) {
  lexer->pos += ns;
  if (lexer->pos < lexer->len) {
    lexer->p_tok = lexer->source[lexer->pos];
  } else {
    lexer->p_tok = '\0';
  }
}

char lexer_peek(struct Lexer *lexer, int ns) {
  if (lexer->pos + ns >= lexer->len) {
    return '\0';
  }
  return lexer->source[lexer->pos + ns];
}

char lexer_eat(struct Lexer *lexer) {
  char c = lexer->p_tok;
  lexer_advance(lexer, 1);
  return c;
}

void lexer_parse_int_literal(struct Lexer *lexer) {
    char *literal = (char*)malloc(MAX_INT_LEN);
    if (!literal) {
        perror("Failed to allocate memory for integer literal");
        return;
    }
    literal[0] = '\0';
    ALLOC_TOKEN(token);
    token->pos = lexer->pos;
    token->value = literal;
    token->type = TOKEN_TYPE_INTEGER;

    for (char c = lexer_eat(lexer); lexer_utils_isnum(c); c = lexer_eat(lexer)) {
        strncat(literal, &c, 1);
    }
    lexer_advance(lexer, -1);

    token->len = strlen(literal);
    CLEANUP_SIZE(token);

    lexer_append_token(lexer, token);
}

void lexer_parse_multi_ps(struct Lexer *lexer) {
    char *literal = (char*)malloc(MAX_IDENT_LEN);
    if (!literal) {
        perror("Failed to allocate memory for identifier");
        return;
    }
    literal[0] = '\0';

    ALLOC_TOKEN(token);
    token->pos = lexer->pos;
    token->value = literal;
    token->type = TOKEN_TYPE_IDENTIFIER;

    for (char c = lexer_eat(lexer); lexer_utils_isalpha(c); c = lexer_eat(lexer)) {
        strncat(literal, &c, 1);
    }
    lexer_advance(lexer, -1);

    token->len = strlen(literal);
    CLEANUP_SIZE(token);

    if (lexer_utils_iskeyword(literal)) {
        token->type = TOKEN_TYPE_KEYWORD;
    }
    else {
      struct macro_stream *bmacro = lexer_utils_ismacro(lexer, literal);
      if (bmacro) {
        pre_process_macroref(lexer, token, bmacro);
        return;
      }
    }

    lexer_append_token(lexer, token);
}

void lexer_parse_prefix(struct Lexer *lexer) {
    int plen = lexer_utils_isprefix(lexer);
    char *literal = (char*)malloc(plen + 1);
    if (!literal) {
        perror("Failed to allocate memory for prefix");
        return;
    }
    literal[0] = '\0';

    ALLOC_TOKEN(token);
    token->pos = lexer->pos;
    token->value = literal;
    token->type = TOKEN_TYPE_PREFIX;

    for (int i = 0; i < plen; i++) {
        literal[i] = lexer_eat(lexer);
    }
    literal[plen] = '\0';

    token->len = plen;
    CLEANUP_SIZE(token);

    if (strcmp(literal, PREFIXES[MACRO_DEF_INDEX]) == 0) {
      pre_process_macrodef(lexer);
      return;
    }

    lexer_append_token(lexer, token);
}

void lexer_parse_symbol(struct Lexer *lexer) {
    struct symbol_entry *symbol = lexer_utils_get_symbol(lexer);
    if (!symbol) {
        WARN("Unknown symbol: %c\n", lexer->p_tok);
        lexer_advance(lexer, 1);
        return;
    }

    ALLOC_TOKEN(token);
    token->pos = lexer->pos;
    token->value = (char*)malloc(strlen(symbol->value) + 1);
    if (!token->value) {
        perror("Failed to allocate memory for symbol token");
        free(token);
        return;
    }

    strcpy(token->value, symbol->value);
    token->type = symbol->type;
    token->len = strlen(symbol->value);

    lexer_advance(lexer, token->len);
    lexer_append_token(lexer, token);
}

void lexer_discriminator(struct Lexer *lexer) {
  if (lexer_utils_isnum(lexer->p_tok)) {
    lexer_parse_int_literal(lexer);
  }
  else if (lexer_utils_isprefix(lexer)) {
    lexer_parse_prefix(lexer);
  }
  else if (lexer_utils_isalpha(lexer->p_tok)) {
    lexer_parse_multi_ps(lexer);
  }
  else if (lexer->p_tok == '/' && lexer_peek(lexer, 1) == '/') {
    lexer_advance(lexer, 2);
    while (lexer->p_tok != '\n') {
      lexer_advance(lexer, 1);
    }
  }
  else if (lexer->p_tok == '/' && lexer_peek(lexer, 1) == '*') {
    lexer_advance(lexer, 2);
    while (lexer->p_tok != '*' && lexer_peek(lexer, 1) != '/') {
      lexer_advance(lexer, 1);
    }
    lexer_advance(lexer, 2);
  }
  else if (lexer_utils_issymbol(lexer)) {
    lexer_parse_symbol(lexer);
  }
  else if (lexer->p_tok == ' ' || lexer->p_tok == '\n' || lexer->p_tok == '\t' || lexer->p_tok == '\r' || lexer->p_tok == '\0') {
    lexer_advance(lexer, 1); // Handle whitespaces
  }
  else {
    WARN("Unknown symbol: %c\n", lexer->p_tok);
    lexer_advance(lexer, 1);
  }
}

void lexer_parse(struct Lexer *lexer) {
  while (lexer->pos < lexer->len) {
    lexer_discriminator(lexer);
  }
}

void lexer_destroy(struct Lexer *lexer) {
  struct token_stream *ts = lexer->tokens_begin;
  while (ts) {
    struct token_stream *next = ts->next;
    free(ts->token->value);
    free(ts->token);
    free(ts);
    ts = next;
  }

  struct macro_stream *ms = lexer->macros;
  while (ms) {
    struct macro_stream *next = ms->next;
    free(ms->name);
    free(ms->value);
    free(ms);
    ms = next;
  }

  free(lexer);
}

void __lexer__dump_tokens(struct Lexer *lexer) {
  struct token_stream *ts = lexer->tokens_begin;
  while (ts) {
    DEBUG("Token: [%d] '%s' (%d)\n", ts->token->type, ts->token->value, ts->token->len);
    ts = ts->next;
  }
}

void __lexer_dump_macros(struct Lexer *lexer) {
  struct macro_stream *ms = lexer->macros;
  while (ms) {
    DEBUG("Macro: %s = %s\n", ms->name, ms->value);
    ms = ms->next;
  }
}

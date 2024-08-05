#include <lexer/lexer.h>
#include <parser/parser.h>
#include <pool/token.h>
#include <pool/log.h>
#include <reporter/report.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>

void fmt_string(char *buffer, size_t size, const char *format, ...) {
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, size, format, args);
    va_end(args);
}

void error_dump(token_t *token, struct Lexer *lexer, const char *msg) {
  int line = 1;
  int col = 1;
  char *line_str = malloc(256);
  for (unsigned i = 0; i < token->pos; i++) {
    if (lexer->source[i] == '\n') {
      line++;
      col = 1;
    } else {
      col++;
    }
  }
  int cline=1;
  int linep=0;
  for (unsigned i=0; i < lexer->len; i++) {
    if (lexer->source[i] == '\n') {
      cline++;
    }
    if (cline == line) {
      line_str[linep++] = lexer->source[i+1];
    }
    if (cline>line) {
      line_str[linep-1] = '\0';
      break;
    }
  }

  ERROR("%s. At line %d, col %d:\n", msg, line, col);

  char line_dmp[256];
  fmt_string(line_dmp, sizeof(line_dmp), "    [%d] |  %s\n", line, line_str);

  uint llen = strlen(line_str);
  uint ldlen = strlen(line_dmp);
  uint offset = ldlen - llen;

  PRINT("    \x1b[34m[%d] |  \x1b[36m%s\n\x1b[0m", line, line_str);
  for (uint i = 0; i < col + offset; i++) {
    PRINT(" ");
  }
  for (uint i = 0; i < token->len; i++) {
    PRINT("\x1b[31m^\x1b[0m");
  }
  PRINT("\n");
}

void report_error(token_t *token, struct Lexer *lexer, const char *message) {
  error_dump(token, lexer, message);
  exit(0);
}
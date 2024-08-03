#include <lexer/lexer.h>
#include <parser/parser.h>
#include <pool/token.h>
#include <pool/log.h>
#include <reporter/report.h>
#include <stdlib.h>


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
  PRINT("    \x1b[34m[%d] |  \x1b[36m%s\n\x1b[0m", line, line_str);
}

void report_error(token_t *token, struct Lexer *lexer, const char *message) {
  error_dump(token, lexer, message);
  exit(0);
}
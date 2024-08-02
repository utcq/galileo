#include <pool/log.h>
#include <lexer/lexer.h>
#include <stdio.h>
#include <stdlib.h>

void lex_file() {
  const char buffer[] = "m!def VAL_TEST 10";
  unsigned len = sizeof(buffer);

  struct Lexer *lexer = lexer_create((char*)buffer, len);
  lexer_parse(lexer);
  __lexer__dump_tokens(lexer);
  __lexer_dump_macros(lexer);
}

int __test_main(void) {
  lex_file();
  return 0;
}

#include <pool/log.h>
#include <lexer/lexer.h>

int __test_main(void) {
  const char input[] = "f!int add() {}";
  struct Lexer *lexer = lexer_create((char*)input, sizeof(input) - 1);
  lexer_parse(lexer);
  __lexer__dump_tokens(lexer);
  return 0;
}

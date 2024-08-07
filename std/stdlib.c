#include <stdio.h>

void test_ffi() {
  int a=5;
  int b=6;
  int c=a+b;
  printf("Hello Hack Club, I'm a FFI, I'm not defined at compile time, I'm defined at runtime!!\nCrazy, right?\n");
}

void test_print(char *str) {
  printf("%s", str);
}
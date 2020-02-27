#ifndef SSQL_TEST_H
#define SSQL_TEST_H

#include <stdio.h>

#define ok(result) { \
  if ((result) == 0) printf("%s - \033[0;31mFAILED\033[0m\n", #result); \
  else printf("%s - \033[0;32mok\033[0m\n", #result); \
}

void SSQL_TestCreateTable();

#endif

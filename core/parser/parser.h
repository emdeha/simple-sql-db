#ifndef SSQL_Parser_h
#define SSQL_Parser_h

#include "../../lib/mpc/mpc.h"

typedef struct {
  mpc_parser_t *main;
  mpc_parser_t **parsers;
  size_t parsersLength;
} Parser;

typedef struct {
  mpc_result_t parseResult;
  int success;
} ParseResult;

Parser* SSQL_ParserInit();
void SSQL_ParserCleanUp(Parser *p);
ParseResult* SSQL_ParserParse(Parser *p);

#endif

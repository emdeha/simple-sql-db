#include "./lib/mpc/mpc.h"
#include "./core/parser/parser.h"
#include "./core/execution-tree/execution-tree.h"
#include "./core/executor/executor.h"
#include "./core/loader/loader.h"

/*
 * Main
 */
int main() {
  Parser *p = SSQL_ParserInit();

  ParseResult *r = SSQL_ParserParse(p);

  if (r->success) {
    mpc_ast_print(r->parseResult.output);
    printf("\n\n");

    Schema *s = SSQL_LoadSchema();

    ExecutionTree *executionTree = SSQL_CreateExecutionTree(r->parseResult.output, s);
    printf("exec tree done: %i\n", executionTree->operation);
    printf("left: %i\n", executionTree->left->argument.type);
    printf("left: %s\n", executionTree->left->argument.charData);
    Arg arg = executionTree->right->argument;
    printf("right (type): %i\n", arg.type);
    printf("right (rowNum): %lu\n", arg.rowNum);
    printf("right (valueNum): %lu\n", arg.relationRowData[0].valueNum);
    for (size_t j = 0; j < arg.rowNum; j++) {
      printf("  row %lu\n", j);
      for (size_t i = 0; i < arg.relationRowData[j].valueNum; i++) {
        RelationValue val = arg.relationRowData[j].values[i];
        if (val.type.name == integer_t) {
          printf("    right (value %lu): %d\n", i, val.integer);
        } else {
          printf("    right (value %lu): %s\n", i, val.str);
        }
      }
    }

    SSQL_ExecuteTree(executionTree, s);

    mpc_ast_delete(r->parseResult.output);
    SSQL_CleanUpExecutionTree(executionTree);
    SSQL_CleanUpSchema(s);
  } else {
    mpc_err_print(r->parseResult.error);
    mpc_err_delete(r->parseResult.error);
  }

  free(r);
  SSQL_ParserCleanUp(p);

  return 0;
}

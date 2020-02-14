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

    ExecutionTree *executionTree = SSQL_CreateExecutionTree(r->parseResult.output);
    printf("exec tree done: %i\n", executionTree->operation);
    printf("left: %i\n", executionTree->left->argument.type);
    printf("left: %s\n", executionTree->left->argument.charData);
    printf("right: %i\n", executionTree->right->argument.type);
    printf("right: %lu\n", executionTree->right->argument.relationRowData[0].valueNum);
    for (size_t i = 0; i < executionTree->right->argument.relationRowData[0].valueNum; i++) {
      printf("right: %d\n", executionTree->right->argument.relationRowData[0].values[i].integer);
    }

    Schema *s = SSQL_LoadSchema();
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

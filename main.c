#include "./lib/mpc/mpc.h"
#include "./core/parser/parser.h"
#include "./core/execution-tree/execution-tree.h"
#include "./core/executor/executor.h"

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
    printf("right: %s\n", executionTree->right->argument.relationColumnData[0].attribute);
    printf("right: %i\n", executionTree->right->argument.relationColumnData[0].type.name);

    SSQL_ExecuteTree(executionTree);

    mpc_ast_delete(r->parseResult.output);
    SSQL_CleanUpExecutionTree(executionTree);
  } else {
    mpc_err_print(r->parseResult.error);
    mpc_err_delete(r->parseResult.error);
  }

  free(r);
  SSQL_ParserCleanUp(p);

  return 0;
}

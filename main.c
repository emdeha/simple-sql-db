#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "./lib/mpc/mpc.h"
#include "./core/parser/parser.h"
#include "./core/execution-tree/execution-tree.h"

/*
 * Execution
 */
void executeCreateTable(ExecutionTree *execTree) {
  assert(execTree->left->argument.type == RELATION_NAME);
  assert(execTree->right->argument.type == RELATION_DEFINITION);

  printf("asserted\n");

  FILE *fp;

  // TODO: Check if relation already exists. If it does, throw an error.
  // TODO: Open the file for appending.
  if ((fp = fopen("./db-data/schema", "w")) == NULL) {
    fprintf(stderr, "can't open schema file");
    return;
  }

  fprintf(fp, "%s\t", execTree->left->argument.charData);
  for (int i = 0; i < execTree->right->argument.columnNum; i++) {
    RelationColumn column = execTree->right->argument.relationColumnData[i];
    fprintf(fp, "%i\t%lu\t%s\t", column.type.name, column.type.size, column.attribute);
  }
  fprintf(fp, "\n");

  fclose(fp);
}

void executeTree(ExecutionTree *execTree) {
  if (execTree->operation == CREATE_TABLE) {
    executeCreateTable(execTree);
  } else {
    printf("Operation not implemented: %i\n", execTree->operation);
  }
}

/*
 * Main
 */
int main() {
  Parser *p = SSQL_ParserInit();

  ParseResult *r = SSQL_ParserParse(p);

  if (r->success) {
    mpc_ast_print(r->parseResult.output);
    printf("\n\n");

    ExecutionTree *executionTree = createExecutionTree(r->parseResult.output);
    printf("exec tree done: %i\n", executionTree->operation);
    printf("left: %i\n", executionTree->left->argument.type);
    printf("left: %s\n", executionTree->left->argument.charData);
    printf("right: %i\n", executionTree->right->argument.type);
    printf("right: %s\n", executionTree->right->argument.relationColumnData[0].attribute);
    printf("right: %i\n", executionTree->right->argument.relationColumnData[0].type.name);

    executeTree(executionTree);

    mpc_ast_delete(r->parseResult.output);
    cleanUpExecutionTree(executionTree);
  } else {
    mpc_err_print(r->parseResult.error);
    mpc_err_delete(r->parseResult.error);
  }

  free(r);
  SSQL_ParserCleanUp(p);

  return 0;
}

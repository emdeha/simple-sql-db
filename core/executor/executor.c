#include <assert.h>

#include "executor.h"

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

void SSQL_ExecuteTree(ExecutionTree *execTree) {
  if (execTree->operation == CREATE_TABLE) {
    executeCreateTable(execTree);
  } else {
    printf("Operation not implemented: %i\n", execTree->operation);
  }
}

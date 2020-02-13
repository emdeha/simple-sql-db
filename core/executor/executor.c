#include <assert.h>

#include "executor.h"

// TODO: Add error handling
void putString(FILE *fp, char *string) {
  fprintf(fp, "%lu\t%s\t", strlen(string), string);
}

void putInt(FILE *fp, int integer) {
  fprintf(fp, "%d\t", integer);
}

void putSizeT(FILE *fp, size_t sizeT) {
  fprintf(fp, "%lu\t", sizeT);
}

void executeCreateTable(ExecutionTree *execTree) {
  assert(execTree->left->argument.type == RELATION_NAME);
  assert(execTree->right->argument.type == RELATION_DEFINITION);

  printf("asserted\n");

  FILE *fp;

  // TODO: Check if relation already exists. If it does, throw an error.
  // TODO: Open the file for appending.
  // TODO: Get file name from config.
  if ((fp = fopen("./db-data/schema", "w")) == NULL) {
    fprintf(stderr, "can't open schema file");
    return;
  }

  putString(fp, execTree->left->argument.charData);
  putInt(fp, execTree->right->argument.columnNum);

  for (int i = 0; i < execTree->right->argument.columnNum; i++) {
    RelationColumn column = execTree->right->argument.relationColumnData[i];
    putInt(fp, column.type.name);
    putSizeT(fp, column.type.size);
    putString(fp, column.attribute);
  }

  fclose(fp);
}

void SSQL_ExecuteTree(ExecutionTree *execTree) {
  if (execTree->operation == CREATE_TABLE) {
    executeCreateTable(execTree);
  } else {
    printf("Operation not implemented: %i\n", execTree->operation);
  }
}

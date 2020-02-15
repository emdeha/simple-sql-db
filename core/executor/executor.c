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

void executeCreateTable(ExecutionTree *execTree, Schema *schema) {
  assert(execTree->left->argument.type == RELATION_NAME);
  assert(execTree->right->argument.type == RELATION_DEFINITION);

  // TODO: Offload this to the preprocessor
  for (size_t i = 0; i < schema->relationNum; i++) {
    // Checks whether a schema with the same name already exists
    if (strcmp(schema->relations[i].relationName, execTree->left->argument.charData) == 0) {
      fprintf(stderr, "schema with the [%s] name already exists\n",
        schema->relations[i].relationName);
      return;
    }
  }

  FILE *fp;

  // TODO: Open the file for appending.
  // TODO: Get file name from config.
  if ((fp = fopen("./db-data/schema", "w")) == NULL) {
    fprintf(stderr, "can't open schema file");
    return;
  }

  // Currently we only support one relation per schema, so we hard-code this
  putSizeT(fp, 1);

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

void executeInsertInto(ExecutionTree *execTree) {
  assert(execTree->left->argument.type == RELATION_NAME);
  assert(execTree->right->argument.type == RELATION_VALUES);

  FILE *fp;

  if ((fp = fopen("./db-data/relations", "a")) == NULL) {
    fprintf(stderr, "can't open relations file");
    return;
  }

  putString(fp, execTree->left->argument.charData);
  putInt(fp, execTree->right->argument.rowNum);

  for (size_t rowIdx = 0; rowIdx < execTree->right->argument.rowNum; rowIdx++) {
    RelationRow row = execTree->right->argument.relationRowData[rowIdx];
    putInt(fp, row.valueNum);
    for (size_t valueIdx = 0; valueIdx < row.valueNum; valueIdx++) {
      RelationValue val = row.values[valueIdx];
      if (val.type.name == integer_t) {
        putInt(fp, val.integer);
      } else {
        putString(fp, val.str);
      }
    }
  }

  fclose(fp);
}

void SSQL_ExecuteTree(ExecutionTree *execTree, Schema *schema) {
  switch (execTree->operation) {
    case CREATE_TABLE:
      executeCreateTable(execTree, schema);
      break;
    case INSERT_INTO:
      executeInsertInto(execTree);
      break;
    default:
      fprintf(stderr, "Operation not implemented: %i\n", execTree->operation);
      break;
  }
}

#include "loader.h"

#include <stdio.h>

// TODO: Add error checking
size_t getSizeT(FILE *fp) {
  size_t sizeT = 0;
  fscanf(fp, "%lu\t", &sizeT);
  return sizeT;
}

char* getString(FILE *fp) {
  size_t stringSize = getSizeT(fp) + 1;
  char *str = malloc(sizeof(char) * stringSize);
  fscanf(fp, "%s\t", str);

  return str;
}

int getInt(FILE *fp) {
  int integer = 0;
  fscanf(fp, "%d\t", &integer);
  return integer;
}

Schema* SSQL_LoadSchema() {
  FILE *fp;

  // TODO: Get file name from config.
  if ((fp = fopen("./db-data/schema", "r")) == NULL) {
    fprintf(stderr, "can't open schema file");
    return NULL;
  }

  Schema *schema = malloc(sizeof(Schema));

  schema->relationNum = getSizeT(fp);
  schema->relations = malloc(sizeof(Relation) * schema->relationNum);
  for (size_t relIdx = 0; relIdx < schema->relationNum; relIdx++) {
    schema->relations[relIdx].relationName = getString(fp);
    schema->relations[relIdx].columnNum = getSizeT(fp);
    schema->relations[relIdx].relationColumns =
      malloc(sizeof(RelationColumn) * schema->relations[relIdx].columnNum);
    for (size_t colIdx = 0; colIdx < schema->relations[relIdx].columnNum; colIdx++) {
      schema->relations[relIdx].relationColumns[colIdx].type.name = getInt(fp);
      schema->relations[relIdx].relationColumns[colIdx].type.size = getSizeT(fp);
      schema->relations[relIdx].relationColumns[colIdx].attribute = getString(fp);
    }
  }

  fclose(fp);

  return schema;
}

void SSQL_PrintSchema(Schema *s) {
  for (size_t relIdx = 0; relIdx < s->relationNum; relIdx++) {
    printf("relationName %s\n", s->relations[relIdx].relationName);
    printf("columnNum %lu\n", s->relations[relIdx].columnNum);
    for (size_t colIdx = 0; colIdx < s->relations[relIdx].columnNum; colIdx++) {
      printf("  column %lu\n", colIdx);
      printf("    typeName %d\n", s->relations[relIdx].relationColumns[colIdx].type.name);
      printf("    typeSize %lu\n", s->relations[relIdx].relationColumns[colIdx].type.size);
      printf("    attribute %s\n", s->relations[relIdx].relationColumns[colIdx].attribute);
    }
  }
}

void SSQL_CleanUpSchema(Schema *s) {
  for (size_t relIdx = 0; relIdx < s->relationNum; relIdx++) {
    free(s->relations[relIdx].relationName);
    for (size_t colIdx = 0; colIdx < s->relations[relIdx].columnNum; colIdx++) {
      free(s->relations[relIdx].relationColumns[colIdx].attribute);
    }
    free(s->relations[relIdx].relationColumns);
  }
  free(s->relations);
  free(s);
}

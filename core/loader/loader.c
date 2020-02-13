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
  schema->relationName = getString(fp);
  schema->columnNum = getSizeT(fp);
  schema->relationColumns = malloc(sizeof(RelationColumn) * schema->columnNum);
  for (size_t i = 0; i < schema->columnNum; i++) {
    schema->relationColumns[i].type.name = getInt(fp);
    schema->relationColumns[i].type.size = getSizeT(fp);
    schema->relationColumns[i].attribute = getString(fp);
  }

  fclose(fp);

  return schema;
}

void SSQL_PrintSchema(Schema *s) {
  printf("relationName %s\n", s->relationName);
  printf("columnNum %lu\n", s->columnNum);
  for (size_t i = 0; i < s->columnNum; i++) {
    printf("  column %lu\n", i);
    printf("    typeName %d\n", s->relationColumns[i].type.name);
    printf("    typeSize %lu\n", s->relationColumns[i].type.size);
    printf("    attribute %s\n", s->relationColumns[i].attribute);
  }
}

void SSQL_CleanUpSchema(Schema *s) {
  free(s->relationName);
  for (size_t i = 0; i < s->columnNum; i++) {
    free(s->relationColumns[i].attribute);
  }
  free(s->relationColumns);
  free(s);
}

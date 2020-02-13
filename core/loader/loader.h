#ifndef SSQL_Loader_h
#define SSQL_Loader_h

#include "../relation-column.h"

typedef struct {
  char *relationName;
  RelationColumn *relationColumns;
  size_t columnNum;
} Relation;

typedef struct {
  Relation *relations;
  size_t relationNum;
} Schema;

Schema* SSQL_LoadSchema();
void SSQL_PrintSchema(Schema *s);
void SSQL_CleanUpSchema(Schema *s);

#endif

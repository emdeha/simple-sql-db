#ifndef SSQL_RelationColumn_h
#define SSQL_RelationColumn_h

#include <stdlib.h>

typedef struct {
  enum TypeName { integer_t, char_t } name;
  size_t size;
} Type;

typedef struct {
  char *attribute;
  Type type;
} RelationColumn;

#endif

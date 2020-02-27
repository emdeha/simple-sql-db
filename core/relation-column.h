#ifndef SSQL_RelationColumn_h
#define SSQL_RelationColumn_h

#include <stdlib.h>

typedef enum {
  integer_t,
  char_t,
  invalid_t = -1
} TypeName;

typedef struct {
  TypeName name;
  size_t size;
} Type;

typedef struct {
  char *attribute;
  Type type;
} RelationColumn;

#endif

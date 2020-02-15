#ifndef SSQL_ExecutionTree_h
#define SSQL_ExecutionTree_h

#include "../../lib/mpc/mpc.h"
#include "../relation-column.h"
#include "../loader/loader.h"

typedef enum { CREATE_TABLE, INSERT_INTO, SELECT } Op;

typedef enum { RELATION_NAME, RELATION_DEFINITION, RELATION_VALUES } ArgType;

typedef struct {
  Type type;
  union {
    int integer;
    char *str;
  };
} RelationValue;

typedef struct {
  RelationValue *values;
  size_t valueNum;
} RelationRow;

typedef struct {
  ArgType type;
  union {
    char *charData;
    struct {
      RelationColumn *relationColumnData;
      // TODO: This should be size_t
      int columnNum;
    };
    struct {
      RelationRow *relationRowData;
      size_t rowNum;
    };
  };
} Arg;

typedef struct ExecutionTree {
  union {
    Op operation;
    Arg argument;
  };
  int isOperation;
  struct ExecutionTree *left;
  struct ExecutionTree *right;
} ExecutionTree;

ExecutionTree* SSQL_CreateExecutionTree(mpc_ast_t *ast, Schema *s);
void SSQL_CleanUpExecutionTree(ExecutionTree *execTree);

#endif

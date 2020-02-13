#ifndef SSQL_ExecutionTree_h
#define SSQL_ExecutionTree_h

#include "../../lib/mpc/mpc.h"
#include "../relation-column.h"

typedef enum { CREATE_TABLE, SELECT } Op;

typedef enum { RELATION_NAME, RELATION_DEFINITION } ArgType;

typedef struct {
  ArgType type;
  union {
    char *charData;
    struct {
      RelationColumn *relationColumnData;
      int columnNum;
    };
  };
} Arg;

typedef struct ExecutionTree {
  union {
    Op operation;
    Arg argument;
  };
  struct ExecutionTree *left;
  struct ExecutionTree *right;
} ExecutionTree;

ExecutionTree* SSQL_CreateExecutionTree(mpc_ast_t *ast);
void SSQL_CleanUpExecutionTree(ExecutionTree *execTree);

#endif

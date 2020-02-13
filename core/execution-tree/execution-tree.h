#ifndef SSQL_ExecutionTree_h
#define SSQL_ExecutionTree_h

#include "../../lib/mpc/mpc.h"

typedef struct {
  enum TypeName { integer_t, char_t } name;
  size_t size;
} Type;

typedef struct {
  char *attribute;
  Type type;
} RelationColumn;

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

ExecutionTree* createExecutionTree(mpc_ast_t *ast);
void cleanUpExecutionTree(ExecutionTree *execTree);

#endif

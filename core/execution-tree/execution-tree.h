#ifndef SSQL_ExecutionTree_h
#define SSQL_ExecutionTree_h

#include "../../lib/mpc/mpc.h"
#include "../relation-column.h"
#include "../loader/loader.h"

typedef enum {
  CREATE_TABLE,
  INSERT_INTO,
  PROJECT,
  CONDITION
} Op;

typedef enum {
  RELATION_NAME,
  RELATION_DEFINITION,
  RELATION_VALUES,
  PROJECT_ATTRIBUTES,
  CONDITION_EXPRESSION,
  RELATIONS_LIST
} ArgType;

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

typedef enum {
  INVALID = -1,
  AND,
  OR,
  IN,
  LIKE,
  EQ,
  ATTRIBUTE,
  PATTERN,
  VALUE,
} ConditionOperand;

// TODO: Add handling for subqueries
typedef struct ConditionExpression {
  union {
    char *attributeName;
    char *pattern;
    // TODO: Add separate value types
    char *value;
  };

  ConditionOperand op;

  struct ConditionExpression *left;
  struct ConditionExpression *right;
} ConditionExpression;

typedef struct {
  ArgType type;
  union {
    // RELATION_NAME
    char *charData;
    // RELATION_DEFINITION
    struct {
      RelationColumn *relationColumnData;
      // TODO: This should be size_t
      int columnNum;
    };
    // RELATION_VALUES
    struct {
      RelationRow *relationRowData;
      size_t rowNum;
    };
    // PROJECT_ATTRIBUTES
    struct {
      char **attributesData;
      size_t attributeNum;
    };
    // CONDITION_EXPRESSION
    ConditionExpression *conditionExpression;
    // RELATIONS_LIST
    struct {
      Relation *relations;
      size_t relationNum;
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

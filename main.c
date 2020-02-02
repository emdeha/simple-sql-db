#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "./lib/mpc/mpc.h"
#include "./core/parser/parser.h"

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

Type getType(mpc_ast_t *astType) {
  Type t;

  if (astType->children_num == 0) {
    t.name = integer_t;
    t.size = 0;
  } else {
    t.name = char_t;
    t.size = atoi(astType->children[1]->contents);
  }

  return t;
}

RelationColumn* extractRelationColumns(mpc_ast_t *astRelationColumns, int *columnNum) {
  RelationColumn *columns =
    malloc(sizeof(RelationColumn) * (astRelationColumns->children_num / 2));

  int j = 0;

  for (int i = 1; i < astRelationColumns->children_num; i += 2) {
    mpc_ast_t *column = astRelationColumns->children[i];

    if (strcmp(column->contents, ")") == 0) break;

    columns[j].attribute = column->children[0]->contents;
    columns[j].type = getType(column->children[1]);
    j++;
  }

  *columnNum = j;

  return columns;
}

void fillCreateTableExecutionTree(ExecutionTree *execTree, mpc_ast_t **ast, mpc_ast_trav_t **trav) {
  while (*ast != NULL) {
    if (strcmp((*ast)->tag, "table_name|regex") == 0) {
      ExecutionTree *left = malloc(sizeof(ExecutionTree));
      left->argument.type = RELATION_NAME;
      left->argument.charData = (*ast)->contents;
      left->right = NULL;
      left->left = NULL;

      execTree->left = left;
    } else if (strcmp((*ast)->tag, "table_data|>") == 0) {
      ExecutionTree *right = malloc(sizeof(ExecutionTree));
      right->argument.type = RELATION_DEFINITION;
      int columnNum = 0;
      right->argument.relationColumnData = extractRelationColumns(*ast, &columnNum);
      right->argument.columnNum = columnNum;
      right->right = NULL;
      right->left = NULL;

      execTree->right = right;
    }

    *ast = mpc_ast_traverse_next(&(*trav));
  }
}

ExecutionTree createExecutionTree(mpc_ast_t *ast) {
  mpc_ast_trav_t *trav = mpc_ast_traverse_start(ast, mpc_ast_trav_order_pre);
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&trav);

  ExecutionTree execTree;
  execTree.left = NULL;
  execTree.right = NULL;

  while (ast_next != NULL) {
    if (strcmp(ast_next->tag, "create|>") == 0) {
      execTree.operation = CREATE_TABLE;
      fillCreateTableExecutionTree(&execTree, &ast_next, &trav);
      if (ast_next == NULL) {
        // fillCreateTableExecutionTree may have reached the end of the
        // traversal, so we shouldn't push it further
        break;
      }
      ast_next = mpc_ast_traverse_next(&trav);
    } else {
      ast_next = mpc_ast_traverse_next(&trav);
    }
  }

  mpc_ast_traverse_free(&trav);

  return execTree;
}

/*
 * Execution
 */
void executeCreateTable(ExecutionTree *execTree) {
  assert(execTree->left->argument.type == RELATION_NAME);
  assert(execTree->right->argument.type == RELATION_DEFINITION);

  printf("asserted\n");

  FILE *fp;

  // TODO: Check if relation already exists. If it does, throw an error.
  // TODO: Open the file for appending.
  if ((fp = fopen("./db-data/schema", "w")) == NULL) {
    fprintf(stderr, "can't open schema file");
    return;
  }

  fprintf(fp, "%s\t", execTree->left->argument.charData);
  for (int i = 0; i < execTree->right->argument.columnNum; i++) {
    RelationColumn column = execTree->right->argument.relationColumnData[i];
    fprintf(fp, "%i\t%lu\t%s\t", column.type.name, column.type.size, column.attribute);
  }
  fprintf(fp, "\n");

  fclose(fp);
}

void executeTree(ExecutionTree *execTree) {
  if (execTree->operation == CREATE_TABLE) {
    executeCreateTable(execTree);
  } else {
    printf("Operation not implemented: %i\n", execTree->operation);
  }
}

/*
 * Main
 */
int main() {
  Parser *p = SSQL_ParserInit();

  ParseResult *r = SSQL_ParserParse(p);

  if (r->success) {
    mpc_ast_print(r->parseResult.output);
    printf("\n\n");

    ExecutionTree executionTree = createExecutionTree(r->parseResult.output);
    printf("exec tree done: %i\n", executionTree.operation);
    printf("left: %i\n", executionTree.left->argument.type);
    printf("left: %s\n", executionTree.left->argument.charData);
    printf("right: %i\n", executionTree.right->argument.type);
    printf("right: %s\n", executionTree.right->argument.relationColumnData[0].attribute);
    printf("right: %i\n", executionTree.right->argument.relationColumnData[0].type.name);

    executeTree(&executionTree);

    mpc_ast_delete(r->parseResult.output);
  } else {
    mpc_err_print(r->parseResult.error);
    mpc_err_delete(r->parseResult.error);
  }

  free(r);
  SSQL_ParserCleanUp(p);

  return 0;
}

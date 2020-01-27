#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "./lib/mpc/mpc.h"

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
    RelationColumn *relationColumnData;
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

RelationColumn* extractRelationColumns(mpc_ast_t *astRelationColumns) {
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
      right->argument.relationColumnData = extractRelationColumns(*ast);
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
  mpc_parser_t *SQL = mpc_new("sql");

  mpc_parser_t *Create = mpc_new("create");
  mpc_parser_t *TableName = mpc_new("table_name");
  mpc_parser_t *TableData = mpc_new("table_data");
  mpc_parser_t *Column = mpc_new("column");
  mpc_parser_t *Type = mpc_new("type");
  mpc_parser_t *TypeChar = mpc_new("type_char");

  mpc_parser_t *Query = mpc_new("query");
  mpc_parser_t *Select = mpc_new("select");
  mpc_parser_t *From = mpc_new("from");
  mpc_parser_t *Where = mpc_new("where");
  mpc_parser_t *Condition = mpc_new("condition");
  mpc_parser_t *Attribute = mpc_new("attribute");
  mpc_parser_t *Relation = mpc_new("relation");
  mpc_parser_t *Pattern = mpc_new("pattern");

  mpca_lang(MPCA_LANG_DEFAULT,
    " sql : (<create> | <query>) ';' ;                                      "
    "                                                                       "
    " create : \"create table\" <table_name> <table_data> ; "
    " "
    " table_name : /[a-zA-Z]+/ ; "
    " "
    " table_data : '(' (<column> ',')* ')' ; "
    " "
    " column : <attribute> <type> ; "
    " "
    " type : \"integer\" | \"date\" | <type_char> ; "
    " "
    " type_char : \"char(\" /\\d+/ ')' ; "
    "                                                                       "
    " query : \"select\" <select>                            "
    "         \"from\" <from>                                  "
    "         <where> ;                                                     "
    "                                                                       "
    " select : <attribute> ',' <select>                                     "
    "        | <attribute> ;                                                "
    "                                                                       "
    " from : <relation> ',' <from>                                          "
    "      | <relation> ;                                                   "
    "                                                                       "
    " where : \"where\" <condition> ;                         "
    "                                                                       "
    " condition : <condition> \"and\" <condition>               "
    "           | <attribute> \"in\" '(' <query> ')'             "
    "           | <attribute> '=' <attribute>                               "
    "           | <attribute> \"like\" <pattern>;              "
    "                                                                       "
    " attribute : /[a-zA-Z0-9]+/;                                           "
    " relation : /[a-zA-Z]+/;                                               "
    " pattern : /'%?\\w+%?'/;                                               ",
  SQL,
  Create, TableName, TableData, Column, Type, TypeChar,
  Query, Select, From, Where, Condition, Attribute, Relation, Pattern, NULL);

  mpc_result_t r;
  if (mpc_parse_pipe("<stdin>", stdin, SQL, &r)) {
    mpc_ast_print(r.output);
    printf("\n\n");

    ExecutionTree executionTree = createExecutionTree(r.output);
    printf("exec tree done: %i\n", executionTree.operation);
    printf("left: %i\n", executionTree.left->argument.type);
    printf("left: %s\n", executionTree.left->argument.charData);
    printf("right: %i\n", executionTree.right->argument.type);
    printf("right: %s\n", executionTree.right->argument.relationColumnData[0].attribute);
    printf("right: %i\n", executionTree.right->argument.relationColumnData[0].type.name);
    printf("right: %s\n", executionTree.right->argument.relationColumnData[1].attribute);
    printf("right: %i\n", executionTree.right->argument.relationColumnData[1].type.name);

    executeTree(&executionTree);

    mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }

  mpc_cleanup(15, SQL,
    Create, TableName, TableData, Column, Type, TypeChar,
    Query, Select, From, Where, Condition, Attribute, Relation, Pattern);

  return 0;
}

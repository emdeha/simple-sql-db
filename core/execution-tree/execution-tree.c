#include "execution-tree.h"

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

/*
  On input:
    insert into a values (1, 2), (3, 4), (5, 6);
      insert|>
        string:1:1 'insert into'
        table_name|regex:1:13 'a'
        string:1:15 'values'
        values|>
          char:1:22 '('
          value|>
            regex:1:23 '1'
            char:1:24 ','
            value|regex:1:26 '2'
          char:1:27 ')'
          char:1:28 ','
          values|>
            char:1:30 '('
            value|>
              regex:1:31 '3'
              char:1:32 ','
              value|regex:1:34 '4'
            char:1:35 ')'
            char:1:36 ','
            values|>
              char:1:38 '('
              value|>
                regex:1:39 '5'
                char:1:40 ','
                value|regex:1:42 '6'
              char:1:43 ')'
      char:1:44 ';'

  Returns:
  [
    RelationRow({
      values: [
        RelationValue({
          type: integer_t,
          integer: 2
        }),
        RelationValue({
          type: integer_t,
          integer: 2
        })
      ],
      valueNum: 2
    }),
    RelationRow({
      values: [
        RelationValue({
          type: integer_t,
          integer: 3
        }),
        RelationValue({
          type: integer_t,
          integer: 4
        })
      ],
      valueNum: 2
    }),
    RelationRow({
      values: [
        RelationValue({
          type: integer_t,
          integer: 5
        }),
        RelationValue({
          type: integer_t,
          integer: 6
        })
      ],
      valueNum: 2
    })
  ]
*/
RelationValue* extractValuesForRow(
  mpc_ast_t **ast, mpc_ast_trav_t **trav, size_t *valueNum
) {
  RelationValue *value = malloc(sizeof(RelationValue));
  size_t valueIdx = 0;

  while (*ast != NULL && strcmp((*ast)->tag, "values|>") != 0) {
    while (*ast != NULL &&
        strcmp((*ast)->tag, "values|>") != 0 &&
        strstr((*ast)->tag, "value|") == NULL) {
      *ast = mpc_ast_traverse_next(&(*trav));
    }
    if (*ast != NULL &&
        strstr((*ast)->tag, "value|") != NULL) {
      // TODO: Determine based on schema
      value[valueIdx].integer = atoi((*ast)->children[0]->contents);
      value[valueIdx].type.name = integer_t;
      value[valueIdx].type.size = 0;
      valueIdx++;
      value = realloc(value, sizeof(RelationValue) * (valueIdx + 1));

      *ast = mpc_ast_traverse_next(&(*trav));
    }
  }

  *valueNum = valueIdx + 1;
  return value;
}

RelationRow* extractRelationRows(
  mpc_ast_t **ast, mpc_ast_trav_t **trav, size_t *rowNum
) {
  RelationRow *row = malloc(sizeof(RelationRow));
  size_t rowIdx = 0;

  *ast = mpc_ast_traverse_next(&(*trav));

  while (*ast != NULL) {
    size_t valueNum = 0;
    row[rowIdx].values = extractValuesForRow(ast, trav, &valueNum);
    row[rowIdx].valueNum = valueNum;
    rowIdx++;

    while (*ast != NULL ||
        strcmp((*ast)->tag, "values|>") != 0) {
      *ast = mpc_ast_traverse_next(&(*trav));
    }
    if (*ast != NULL &&
        strcmp((*ast)->tag, "values|>") == 0) {
      row = realloc(row, sizeof(RelationRow) * (rowIdx + 1));

      *ast = mpc_ast_traverse_next(&(*trav));
    }
  }

  *rowNum = rowIdx+1;
  return row;
}

void fillInsertIntoExecutionTree(ExecutionTree *execTree, mpc_ast_t **ast, mpc_ast_trav_t **trav) {
  while (*ast != NULL) {
    if (strcmp((*ast)->tag, "table_name|regex") == 0) {
      ExecutionTree *left = malloc(sizeof(ExecutionTree));
      left->argument.type = RELATION_NAME;
      left->argument.charData = (*ast)->contents;
      left->right = NULL;
      left->left = NULL;

      execTree->left = left;
    } else if (strcmp((*ast)->tag, "values|>") == 0) {
      ExecutionTree *right = malloc(sizeof(ExecutionTree));
      right->argument.type = RELATION_VALUES;
      size_t rowNum = 0;
      right->argument.relationRowData = extractRelationRows(ast, trav, &rowNum);
      right->argument.rowNum = rowNum;
      right->right = NULL;
      right->left = NULL;

      execTree->right = right;
    }

    *ast = mpc_ast_traverse_next(&(*trav));
  }
}

ExecutionTree* SSQL_CreateExecutionTree(mpc_ast_t *ast) {
  mpc_ast_trav_t *trav = mpc_ast_traverse_start(ast, mpc_ast_trav_order_pre);
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&trav);

  ExecutionTree *execTree = malloc(sizeof(ExecutionTree));
  execTree->left = NULL;
  execTree->right = NULL;

  while (ast_next != NULL) {
    if (strcmp(ast_next->tag, "create|>") == 0) {
      execTree->operation = CREATE_TABLE;
      fillCreateTableExecutionTree(execTree, &ast_next, &trav);
      if (ast_next == NULL) {
        // fillCreateTableExecutionTree may have reached the end of the
        // traversal, so we shouldn't push it further
        break;
      }
      ast_next = mpc_ast_traverse_next(&trav);
    } else if (strcmp(ast_next->tag, "insert|>") == 0) {
      execTree->operation = INSERT_INTO;
      fillInsertIntoExecutionTree(execTree, &ast_next, &trav);
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

void SSQL_CleanUpExecutionTree(ExecutionTree *execTree) {
  if (execTree == NULL) {
    return;
  }

  SSQL_CleanUpExecutionTree(execTree->right);
  SSQL_CleanUpExecutionTree(execTree->left);

  if (execTree->argument.type == RELATION_DEFINITION) {
    free(execTree->argument.relationColumnData);
  }

  free(execTree);
}

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
      left->isOperation = 0;
      left->argument.type = RELATION_NAME;
      left->argument.charData = (*ast)->contents;
      left->right = NULL;
      left->left = NULL;

      execTree->left = left;
    } else if (strcmp((*ast)->tag, "table_data|>") == 0) {
      ExecutionTree *right = malloc(sizeof(ExecutionTree));
      right->isOperation = 0;
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
Relation* findRelationByName(Schema *s, char *relationName) {
  for (size_t i = 0; i < s->relationNum; i++) {
    if (strcmp(s->relations[i].relationName, relationName) == 0) {
      return &s->relations[i];
    }
  }

  return NULL;
}

char* removeQuotes(char *str) {
  char *noQuotes = malloc(sizeof(char) * strlen(str) - 1);
  strncpy(noQuotes, str + 1, strlen(str) - 2);
  noQuotes[strlen(str) - 2] = '\0';
  return noQuotes;
}

RelationValue* extractValuesForRow(
  mpc_ast_t **ast, mpc_ast_trav_t **trav, size_t *valueNum,
  Schema *s, char *relationName
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
      Relation *relation = findRelationByName(s, relationName);
      if (!relation) {
        fprintf(stderr, "no relation with name %s\n", relationName);
        return NULL;
      }

      if (valueIdx >= relation->columnNum) {
        fprintf(stderr,
          "valueIdx [%lu] >= columnNum [%lu]\n", valueIdx, relation->columnNum);
        return NULL;
      }

      // if we have 'value|regex: <value>', we won't have children
      char *astValue = (*ast)->children_num > 0
        ? (*ast)->children[0]->contents
        : (*ast)->contents;

      switch (relation->relationColumns[valueIdx].type.name) {
        // TODO: Check if the passed values really correspond to char* and int
        case integer_t: {
          value[valueIdx].integer = atoi(astValue);
          value[valueIdx].type.name = integer_t;
          value[valueIdx].type.size = 0;
          break;
        }
        case char_t: {
          value[valueIdx].str = removeQuotes(astValue);
          value[valueIdx].type.name = char_t;
          value[valueIdx].type.size = relation->relationColumns[valueIdx].type.size;
          break;
        }
        default:
          fprintf(stderr,
            "invalid relation type %i\n", relation->relationColumns[valueIdx].type.name);
          return NULL;
      }

      valueIdx++;
      value = realloc(value, sizeof(RelationValue) * (valueIdx + 1));

      *ast = mpc_ast_traverse_next(&(*trav));
    }
  }

  *valueNum = valueIdx;
  return value;
}

RelationRow* extractRelationRows(
  mpc_ast_t **ast, mpc_ast_trav_t **trav, size_t *rowNum,
  Schema *s, char *relationName
) {
  RelationRow *row = malloc(sizeof(RelationRow));
  size_t rowIdx = 0;

  *ast = mpc_ast_traverse_next(&(*trav));

  while (*ast != NULL) {
    size_t valueNum = 0;
    row[rowIdx].values = extractValuesForRow(ast, trav, &valueNum, s, relationName);
    if (row[rowIdx].values == NULL) {
      return NULL;
    }

    row[rowIdx].valueNum = valueNum;
    rowIdx++;

    while (*ast != NULL &&
        strcmp((*ast)->tag, "values|>") != 0) {
      *ast = mpc_ast_traverse_next(&(*trav));
    }
    if (*ast != NULL &&
        strcmp((*ast)->tag, "values|>") == 0) {
      row = realloc(row, sizeof(RelationRow) * (rowIdx + 1));

      *ast = mpc_ast_traverse_next(&(*trav));
    }
  }

  *rowNum = rowIdx;
  return row;
}

void fillInsertIntoExecutionTree(
  ExecutionTree *execTree, mpc_ast_t **ast, mpc_ast_trav_t **trav, Schema *s
) {
  while (*ast != NULL) {
    if (strcmp((*ast)->tag, "table_name|regex") == 0) {
      ExecutionTree *left = malloc(sizeof(ExecutionTree));
      left->isOperation = 0;
      left->argument.type = RELATION_NAME;
      left->argument.charData = (*ast)->contents;
      left->right = NULL;
      left->left = NULL;

      execTree->left = left;
    } else if (strcmp((*ast)->tag, "values|>") == 0) {
      ExecutionTree *right = malloc(sizeof(ExecutionTree));
      right->isOperation = 0;
      right->argument.type = RELATION_VALUES;
      size_t rowNum = 0;
      right->argument.relationRowData =
        extractRelationRows(ast, trav, &rowNum, s, execTree->left->argument.charData);
      right->argument.rowNum = rowNum;
      right->right = NULL;
      right->left = NULL;

      execTree->right = right;
    }

    *ast = mpc_ast_traverse_next(&(*trav));
  }
}

ExecutionTree* SSQL_CreateExecutionTree(mpc_ast_t *ast, Schema *s) {
  mpc_ast_trav_t *trav = mpc_ast_traverse_start(ast, mpc_ast_trav_order_pre);
  mpc_ast_t *ast_next = mpc_ast_traverse_next(&trav);

  ExecutionTree *execTree = malloc(sizeof(ExecutionTree));
  execTree->left = NULL;
  execTree->right = NULL;

  while (ast_next != NULL) {
    if (strcmp(ast_next->tag, "create|>") == 0) {
      execTree->operation = CREATE_TABLE;
      execTree->isOperation = 1;
      fillCreateTableExecutionTree(execTree, &ast_next, &trav);
      if (ast_next == NULL) {
        // fillCreateTableExecutionTree may have reached the end of the
        // traversal, so we shouldn't push it further
        break;
      }
      ast_next = mpc_ast_traverse_next(&trav);
    } else if (strcmp(ast_next->tag, "insert|>") == 0) {
      execTree->operation = INSERT_INTO;
      execTree->isOperation = 1;
      fillInsertIntoExecutionTree(execTree, &ast_next, &trav, s);
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

  if (!execTree->isOperation) {
    Arg arg = execTree->argument;

    switch (arg.type) {
      case RELATION_DEFINITION:
        free(arg.relationColumnData);
        break;
      case RELATION_NAME:
        break;
      case RELATION_VALUES: {
        for (size_t rowIdx = 0; rowIdx < arg.rowNum; rowIdx++) {
          for (size_t valueIdx = 0; valueIdx < arg.relationRowData[rowIdx].valueNum; valueIdx++) {
            if (arg.relationRowData[rowIdx].values[valueIdx].type.name == char_t) {
              free(arg.relationRowData[rowIdx].values[valueIdx].str);
            }
          }
          free(arg.relationRowData[rowIdx].values);
        }
        free(arg.relationRowData);
        break;
      }
      default:
        fprintf(stderr, "no such arg type %i\n", arg.type);
        break;
    }
  }

  free(execTree);
}

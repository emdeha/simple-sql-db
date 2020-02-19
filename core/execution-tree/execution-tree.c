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

/*
 * Sample tree:
 *
 * select a, b, c from a, b where a = 1 and b = 2 or c like '123';
 * >
 *   query|>
 *     string:1:1 'select'
 *     select|>
 *       attribute|regex:1:8 'a'
 *       char:1:9 ','
 *       attribute|regex:1:11 'b'
 *       char:1:12 ','
 *       attribute|regex:1:14 'c'
 *     string:1:16 'from'
 *     from|>
 *       relation|regex:1:21 'a'
 *       char:1:22 ','
 *       relation|regex:1:24 'b'
 *     where|>
 *       string:1:26 'where'
 *       condition|and_condition_body|>
 *         condition_body|>
 *           attribute|regex:1:32 'a'
 *           char:1:34 '='
 *           value|regex:1:36 '1'
 *         string:1:38 'and'
 *         or_condition_body|>
 *           condition_body|>
 *             attribute|regex:1:42 'b'
 *             char:1:44 '='
 *             value|regex:1:46 '2'
 *           string:1:48 'or'
 *           condition_body|>
 *             attribute|regex:1:51 'c'
 *             string:1:53 'like'
 *             pattern|regex:1:58 ''123''
 *   char:1:63 ';'
 */
char** extractProjectAttributes(
  mpc_ast_t *selectAST, size_t *attributeNum
) {
  *attributeNum = selectAST->children_num / 2 + 1;
  char **attributes = malloc(sizeof(char*) * (*attributeNum));

  for (int i = 0; i < selectAST->children_num; i++) {
    if (strstr(selectAST->children[i]->tag, "attribute") != NULL) {
      attributes[i / 2] = selectAST->children[i]->contents;
    }
  }

  return attributes;
}

ConditionOperand determineConditionOperand(char *operand) {
  if (strcmp(operand, "=") == 0) {
    return EQ;
  } else if (strcmp(operand, "like") == 0) {
    return LIKE;
  }

  fprintf(stderr, "not implemented operand: %s\n", operand);
  return INVALID;
}

ConditionExpression* extractConditionExpression(mpc_ast_t *whereAST) {
  ConditionExpression *expression = malloc(sizeof(ConditionExpression));

  mpc_ast_t *conditionBody = whereAST;
  if (strstr(conditionBody->tag, "and_condition_body") != NULL) {
    expression->op = AND;
    printf("and condition\n");
    // TODO: Check for NULL because this indicates errors
    expression->left = extractConditionExpression(conditionBody->children[0]);
    expression->right = extractConditionExpression(conditionBody->children[2]);
  } else if (strstr(conditionBody->tag, "or_condition_body") != NULL) {
    expression->op = OR;
    printf("or condition\n");
    // TODO: Check for NULL because this indicates errors
    expression->left = extractConditionExpression(conditionBody->children[0]);
    expression->right = extractConditionExpression(conditionBody->children[2]);
  } else if (strstr(conditionBody->tag, "condition_body") != NULL) {
    expression->op = determineConditionOperand(conditionBody->children[1]->contents);

    expression->left = malloc(sizeof(ConditionExpression));
    expression->left->op = ATTRIBUTE;
    expression->left->attributeName = conditionBody->children[0]->contents;
    printf("  attributeName %s\n", expression->left->attributeName);
    expression->left->left = NULL;
    expression->left->right = NULL;

    expression->right = malloc(sizeof(ConditionExpression));
    if (expression->op == EQ) {
      expression->right->op = VALUE;
      expression->right->value = conditionBody->children[2]->contents;
      printf("  value %s\n", expression->right->value);
    } else if (expression->op == LIKE) {
      expression->right->op = PATTERN;
      expression->right->pattern = conditionBody->children[2]->contents;
      printf("  pattern %s\n", expression->right->pattern);
    } else {
      fprintf(stderr, "invalid condition op: %d\n", expression->op);
      return NULL;
    }
    expression->right->left = NULL;
    expression->right->right = NULL;
  } else {
    fprintf(stderr, "invalid condition body: %s\n", conditionBody->tag);
    return NULL;
  }

  return expression;
}

Relation* extractRelations(mpc_ast_t *fromAST, size_t *relationNum, Schema *s) {
  *relationNum = fromAST->children_num / 2 + 1;
  Relation *relations = malloc(sizeof(Relation) * (*relationNum));

  printf("relations\n");

  for (int i = 0; i < fromAST->children_num; i++) {
    if (strstr(fromAST->children[i]->tag, "relation") != NULL) {
      Relation *relation = findRelationByName(s, fromAST->children[i]->contents);
      if (relation == NULL) {
        fprintf(stderr, "no relation with name %s\n", fromAST->children[i]->contents);
        return NULL;
      }

      relations[i / 2] = *relation;
      printf("  relation %s\n", relations[i / 2].relationName);
    }
  }

  return relations;
}

void fillQueryExecutionTree(
  ExecutionTree *execTree, mpc_ast_t **ast, Schema *s
) {
  mpc_ast_t *selectChild = NULL;
  mpc_ast_t *fromChild = NULL;
  mpc_ast_t *whereChild = NULL;

  for (int i = 0; i < (*ast)->children_num; i++) {
    if (strcmp((*ast)->children[i]->tag, "select|>") == 0) {
      selectChild = (*ast)->children[i];
    }
    if (strstr((*ast)->children[i]->tag, "from|") != NULL) {
      fromChild = (*ast)->children[i];
    }
    if (strcmp((*ast)->children[i]->tag, "where|>") == 0) {
      whereChild = (*ast)->children[i];
    }
  }

  execTree->isOperation = 1;
  execTree->operation = PROJECT;
  execTree->right = NULL;

  execTree->left = malloc(sizeof(ExecutionTree));
  execTree->left->isOperation = 0;
  execTree->left->argument.type = PROJECT_ATTRIBUTES;
  execTree->left->argument.attributesData =
    extractProjectAttributes(selectChild, &execTree->left->argument.attributeNum);
  execTree->left->right = NULL;
  printf("extracted attributesData\n");
  for (size_t i = 0; i < execTree->left->argument.attributeNum; i++) {
    printf("  attrib %lu is %s\n", i, execTree->left->argument.attributesData[i]);
  }

  execTree->left->left = malloc(sizeof(ExecutionTree));
  execTree->left->left->isOperation = 1;
  execTree->left->left->operation = CONDITION;
  execTree->left->left->right = NULL;

  execTree->left->left->left = malloc(sizeof(ExecutionTree));
  execTree->left->left->left->isOperation = 0;
  execTree->left->left->left->argument.type = CONDITION_EXPRESSION;
  execTree->left->left->left->argument.conditionExpression =
    extractConditionExpression(whereChild->children[1]);
  execTree->left->left->left->right = NULL;

  execTree->left->left->left->left = malloc(sizeof(ExecutionTree));
  execTree->left->left->left->left->isOperation = 0;
  execTree->left->left->left->left->argument.type = RELATIONS_LIST;
  execTree->left->left->left->left->argument.relations =
    extractRelations(fromChild, &execTree->left->left->left->left->argument.relationNum, s);
  execTree->left->left->left->left->right = NULL;
  execTree->left->left->left->left->left = NULL;
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
        break;
      }
      ast_next = mpc_ast_traverse_next(&trav);
    } else if (strcmp(ast_next->tag, "query|>") == 0) {
      fillQueryExecutionTree(execTree, &ast_next, s);
      if (ast_next == NULL) {
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

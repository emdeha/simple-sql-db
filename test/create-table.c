#include "./test.h"
#include "../core/execution-tree/execution-tree.h"
#include "../core/execution-tree/execution-tree_pi.h"

void test_getType() {
  mpc_ast_t *ast = mpc_ast_new("type|string", "integer");

  ok(getType(ast).name == integer_t && getType(ast).size == 0);

  mpc_ast_delete(ast);

  ast = mpc_ast_new("type|type_char|>", "");
  ast = mpc_ast_add_child(ast, mpc_ast_new("string", "char("));
  ast = mpc_ast_add_child(ast, mpc_ast_new("regex", "50"));
  ast = mpc_ast_add_child(ast, mpc_ast_new("char", ")"));

  ok(getType(ast).name == char_t && getType(ast).size == 50);

  mpc_ast_delete(ast);
}

void test_extractOneRelationColumn() {
  mpc_ast_t *ast = mpc_ast_new("table_data|>", "");
  ast = mpc_ast_add_child(ast, mpc_ast_new("char", "("));

  mpc_ast_t *column = mpc_ast_new("column|>", "");
  column = mpc_ast_add_child(column, mpc_ast_new("attribute|regex", "a"));
  column = mpc_ast_add_child(column, mpc_ast_new("type|string", "integer"));
  ast = mpc_ast_add_child(ast, column);

  ast = mpc_ast_add_child(ast, mpc_ast_new("char", ","));
  ast = mpc_ast_add_child(ast, mpc_ast_new("char", ")"));

  int columnNum = 0;
  RelationColumn *columns = extractRelationColumns(ast, &columnNum);

  ok(columnNum == 1);
  ok(strcmp(columns[0].attribute, "a") == 0);
  ok(columns[0].type.name == integer_t);
  ok(columns[0].type.size == 0);

  mpc_ast_delete(ast);
  free(columns);
}

void test_extractTwoRelationColumns() {
  mpc_ast_t *ast = mpc_ast_new("table_data|>", "");
  ast = mpc_ast_add_child(ast, mpc_ast_new("char", "("));

  mpc_ast_t *columnOne = mpc_ast_new("column|>", "");
  columnOne = mpc_ast_add_child(columnOne, mpc_ast_new("attribute|regex", "a"));
  columnOne = mpc_ast_add_child(columnOne, mpc_ast_new("type|string", "integer"));
  ast = mpc_ast_add_child(ast, columnOne);

  ast = mpc_ast_add_child(ast, mpc_ast_new("char", ","));

  mpc_ast_t *columnTwo = mpc_ast_new("column|>", "");
  columnTwo = mpc_ast_add_child(columnTwo, mpc_ast_new("attribute|regex", "c"));
  mpc_ast_t *columnTwoType = mpc_ast_new("type|type_char|>", "");
  columnTwoType = mpc_ast_add_child(columnTwoType, mpc_ast_new("string", "char("));
  columnTwoType = mpc_ast_add_child(columnTwoType, mpc_ast_new("regex", "50"));
  columnTwoType = mpc_ast_add_child(columnTwoType, mpc_ast_new("char", ")"));

  columnTwo = mpc_ast_add_child(columnTwo, columnTwoType);

  ast = mpc_ast_add_child(ast, columnTwo);

  ast = mpc_ast_add_child(ast, mpc_ast_new("char", ","));
  ast = mpc_ast_add_child(ast, mpc_ast_new("char", ")"));

  int columnNum = 0;
  RelationColumn *columns = extractRelationColumns(ast, &columnNum);

  ok(columnNum == 2);
  ok(strcmp(columns[0].attribute, "a") == 0);
  ok(columns[0].type.name == integer_t);
  ok(columns[0].type.size == 0);
  ok(strcmp(columns[1].attribute, "c") == 0);
  ok(columns[1].type.name == char_t);
  ok(columns[1].type.size == 50);

  mpc_ast_delete(ast);
  free(columns);
}

void test_extractRelationColumns() {
  test_extractOneRelationColumn();
  test_extractTwoRelationColumns();
  // test_extractZeroRelationColumns();
}

/*
 * >
 *  create|>
 *    string:1:1 'create table'
 *    table_name|regex:1:14 'c'
 *    table_data|>
 *      char:1:16 '('
 *      column|>
 *        attribute|regex:1:17 'a'
 *        type|string:1:19 'integer'
 *      char:1:26 ','
 *      column|>
 *        attribute|regex:1:28 'c'
 *        type|type_char|>
 *          string:1:30 'char('
 *          regex:1:35 '50'
 *          char:1:37 ')'
 *      char:1:38 ','
 *      char:1:39 ')'
 *  char:1:40 ';'
 */
void SSQL_TestCreateTable() {
  test_getType();
  test_extractRelationColumns();
}

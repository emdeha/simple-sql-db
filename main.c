#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./lib/mpc/mpc.h"

typedef struct {
  enum TypeName { integer_t, char_t } name;
  size_t size;
} Type;

typedef struct {
  char *attribute;
  Type type;
} RelationColumn;

typedef struct {
  char *name;
  RelationColumn *columns;
  int columnCount;
} Relation;

char* typeToString(Type t) {
  switch (t.name) {
    case integer_t: return "integer_t";
    case char_t: return "char_t";
    default: printf("no such type\n"); exit(1);
  }
}

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
  mpc_ast_print(astRelationColumns);

  RelationColumn *columns =
    malloc(sizeof(RelationColumn) * (astRelationColumns->children_num / 2));

  int j = 0;

  for (int i = 1; i < astRelationColumns->children_num; i += 2) {
    mpc_ast_t *column = astRelationColumns->children[i];

    if (strcmp(column->contents, ")") == 0) break;

    printf("\n\ncolumn\n");
    mpc_ast_print(column);
    printf("\n");

    columns[j].attribute = column->children[0]->contents;
    columns[j].type = getType(column->children[1]);
    j++;
  }

  return columns;
}

void createTable(mpc_ast_t *ast) {
  printf("in create table\n");

  char *tableName = ast->children[1]->contents;
  printf("table name: %s\n", tableName);

  RelationColumn *columns = extractRelationColumns(ast->children[2]);

  Relation relation;
  relation.name = tableName;
  relation.columns = columns;
  relation.columnCount = ast->children[2]->children_num / 2 - 1;

  printf("extracted: %s\n", relation.name);
  printf("\n%d columns\n", relation.columnCount);

  for (int i = 0; i < relation.columnCount; i++) {
    printf("    %s %s %lu\n", relation.columns[i].attribute, typeToString(relation.columns[i].type), relation.columns[i].type.size);
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

    mpc_ast_t *ast = r.output;
    if (strcmp(ast->children[0]->tag, "create|>") == 0) {
      createTable(ast->children[0]);
    } else {
      printf("not implemented: %s\n", ast->children[0]->tag);
    }

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

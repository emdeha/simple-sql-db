#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./lib/mpc/mpc.h"

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

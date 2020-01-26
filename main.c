#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "./lib/mpc/mpc.h"

/*
 * Main
 */
int main() {
  mpc_parser_t *SQL = mpc_new("sql");
  mpc_parser_t *Query = mpc_new("query");
  mpc_parser_t *Select = mpc_new("select");
  mpc_parser_t *From = mpc_new("from");
  mpc_parser_t *Where = mpc_new("where");
  mpc_parser_t *Condition = mpc_new("condition");
  mpc_parser_t *Attribute = mpc_new("attribute");
  mpc_parser_t *Relation = mpc_new("relation");
  mpc_parser_t *Pattern = mpc_new("pattern");

  mpca_lang(MPCA_LANG_DEFAULT,
    " sql : <query> ';' ;                                                   "
    "                                                                       "
    " query : (\"SELECT\" | \"select\") <select>                            "
    "         (\"FROM\" | \"from\") <from>                                  "
    "         <where> ;                                                     "
    "                                                                       "
    " select : <attribute> ',' <select>                                     "
    "        | <attribute> ;                                                "
    "                                                                       "
    " from : <relation> ',' <from>                                          "
    "      | <relation> ;                                                   "
    "                                                                       "
    " where : (\"WHERE\" | \"where\") <condition> ;                         "
    "                                                                       "
    " condition : <condition> (\"AND\" | \"and\") <condition>               "
    "           | <attribute> (\"IN\" | \"in\") '(' <query> ')'             "
    "           | <attribute> '=' <attribute>                               "
    "           | <attribute> (\"LIKE\" | \"like\") <pattern>;              "
    "                                                                       "
    " attribute : /[a-zA-Z0-9]+/;                                           "
    " relation : /[a-zA-Z]+/;                                               "
    " pattern : /'%?\\w+%?'/;                                               ",
  SQL, Query, Select, From, Where, Condition, Attribute, Relation,
  Pattern, NULL);

  mpc_print(SQL);
  mpc_print(Query);
  mpc_print(Select);
  mpc_print(From);
  mpc_print(Where);
  mpc_print(Condition);
  mpc_print(Attribute);
  mpc_print(Relation);
  mpc_print(Pattern);

  mpc_result_t r;
  if (mpc_parse_pipe("<stdin>", stdin, SQL, &r)) {
    mpc_ast_print(r.output);
    mpc_ast_delete(r.output);
  } else {
    mpc_err_print(r.error);
    mpc_err_delete(r.error);
  }

  mpc_cleanup(9, SQL, Query, Select, From, Where, Condition, Attribute, Relation, Pattern);

  return 0;
}

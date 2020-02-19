#include "parser.h"

Parser* SSQL_ParserInit() {
  mpc_parser_t *SQL = mpc_new("sql");

  mpc_parser_t *Create = mpc_new("create");
  mpc_parser_t *TableName = mpc_new("table_name");
  mpc_parser_t *TableData = mpc_new("table_data");
  mpc_parser_t *Column = mpc_new("column");
  mpc_parser_t *Type = mpc_new("type");
  mpc_parser_t *TypeChar = mpc_new("type_char");

  mpc_parser_t *Insert = mpc_new("insert");
  mpc_parser_t *Values = mpc_new("values");
  mpc_parser_t *Value = mpc_new("value");

  mpc_parser_t *Query = mpc_new("query");
  mpc_parser_t *Select = mpc_new("select");
  mpc_parser_t *From = mpc_new("from");
  mpc_parser_t *Where = mpc_new("where");
  mpc_parser_t *Condition = mpc_new("condition");
  mpc_parser_t *ConditionBody = mpc_new("condition_body");
  mpc_parser_t *AndConditionBody = mpc_new("and_condition_body");
  mpc_parser_t *OrConditionBody = mpc_new("or_condition_body");
  mpc_parser_t *Attribute = mpc_new("attribute");
  mpc_parser_t *Relation = mpc_new("relation");
  mpc_parser_t *Pattern = mpc_new("pattern");

  mpca_lang(MPCA_LANG_DEFAULT,
    " sql : (<create> | <query> | <insert>) ';' ;                           "
    "                                                                       "
    " create : \"create table\" <table_name> <table_data> ; "
    " "
    " table_name : /[a-zA-Z]+/ ; "
    " "
    " table_data : '(' (<column> ',')* ')' ; "
    " "
    " column : <attribute> <type> ; "
    " "
    " type : \"integer\" | <type_char> ; "
    " "
    " type_char : \"char(\" /\\d+/ ')' ; "
    "                                                                       "
    " insert : \"insert into\" <table_name> \"values\" <values> ; "
    " "
    " values : '(' <value>* ')' ',' <values> "
    "        | '(' <value>* ')' ; "
    " "
    " value : /'\\w+'/ ',' <value> "
    "       | /\\d+/ ',' <value> "
    "       | /'\\w+'/ "
    "       | /\\d+/ ; "
    " "
    " query : \"select\" <select>                            "
    "         \"from\" <from>                                  "
    "         <where> ;                                                     "
    "                                                                       "
    " select : <attribute> (',' <attribute>)* ;                                     "
    "                                                                       "
    " from : <relation> (',' <relation>)* ;                                          "
    "                                                                       "
    " where : \"where\" <condition> ;                         "
    "                                                                       "
    // " condition : <condition_body> ((\"and\" | \"or\") <condition_body>)* ; "
    " condition : <and_condition_body> "
    "           | <or_condition_body> "
    "           | <condition_body> ; "
    " "
    " and_condition_body : <condition_body> \"and\" "
    "            (<and_condition_body> | <or_condition_body> | <condition_body>)+ ; "
    " "
    " or_condition_body : <condition_body> \"or\" "
    "            (<and_condition_body> | <or_condition_body> | <condition_body>)+ ; "
    " "
    " condition_body: <attribute> \"in\" '(' <query> ')'             "
    "               | <attribute> '=' <value>                              "
    "               | <attribute> \"like\" <pattern>;              "
    "                                                                       "
    " attribute : /[a-zA-Z0-9]+/;                                           "
    " relation : /[a-zA-Z]+/;                                               "
    " pattern : /'%?\\w+%?'/;                                               ",
  SQL,
  Create, TableName, TableData, Column, Type, TypeChar,
  Insert, Values, Value,
  Query, Select, From, Where, Condition, ConditionBody, AndConditionBody, OrConditionBody,
  Attribute, Relation, Pattern, NULL);

  Parser *p = malloc(sizeof(Parser));
  p->main = SQL;
  p->parsers = malloc(20 * sizeof(mpc_parser_t*));
  p->parsers[0] = Create;
  p->parsers[1] = TableName;
  p->parsers[2] = TableData;
  p->parsers[3] = Column;
  p->parsers[4] = Type;
  p->parsers[5] = TypeChar;
  p->parsers[6] = Insert;
  p->parsers[7] = Values;
  p->parsers[8] = Value;
  p->parsers[9] = Query;
  p->parsers[10] = Select;
  p->parsers[11] = From;
  p->parsers[12] = Where;
  p->parsers[13] = Condition;
  p->parsers[14] = ConditionBody;
  p->parsers[15] = AndConditionBody;
  p->parsers[16] = OrConditionBody;
  p->parsers[17] = Attribute;
  p->parsers[18] = Relation;
  p->parsers[19] = Pattern;

  p->parsersLength = 20;
  return p;
}

void SSQL_ParserCleanUp(Parser *p) {
  mpc_cleanup(1, p->main);
  p->main = NULL;
  for (size_t i = 0; i < p->parsersLength; i++) {
    mpc_cleanup(1, p->parsers[i]);
    p->parsers[0] = NULL;
  }
  free(p->parsers);
  p->parsersLength = 0;
  free(p);
}

ParseResult* SSQL_ParserParse(Parser *p) {
  ParseResult *r = malloc(sizeof(ParseResult));

  if (mpc_parse_pipe("<stdin>", stdin, p->main, &r->parseResult)) {
    r->success = 1;
  } else {
    r->success = 0;
  }

  return r;
}

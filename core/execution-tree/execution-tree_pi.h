#ifndef SSQL_EXECUTION_TREE_PI_H
#define SSQL_EXECUTION_TREE_PI_H

#include "./execution-tree.h"

void fillCreateTableExecutionTree(ExecutionTree *execTree, mpc_ast_t **ast, mpc_ast_trav_t **trav);
RelationColumn* extractRelationColumns(mpc_ast_t *astRelationColumns, int *columnNum);
Type getType(mpc_ast_t *astType);

#endif

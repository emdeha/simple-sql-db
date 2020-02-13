#ifndef SSQL_Executor_h
#define SSQL_Executor_h

#include "../execution-tree/execution-tree.h"
#include "../loader/loader.h"

void SSQL_ExecuteTree(ExecutionTree *execTree, Schema *schema);

#endif

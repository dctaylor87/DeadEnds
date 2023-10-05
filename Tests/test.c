#include <stdio.h>
#include "standard.h"
#include "parse.h"
#include "interp.h"
#include "functiontable.h"
#include "pnode.h"

//static bool debugging = false;

extern String currentProgramFileName;
extern int currentProgramLineNumber;
extern FunctionTable *procedureTable;

int main()
{
	//  Parse a simple LifeLines "hello, world" program.
	parseProgram("llprogram", "../Data");


	//  Create a PNProcCall node to call the main procedure with
	currentProgramFileName = "internal";
	currentProgramLineNumber = 1;
	PNode *pnode = procCallPNode("main", null);

	//  Call the main procedure.
	SymbolTable *symbolTable = createSymbolTable();
	PValue returnPvalue;
	interpret(pnode, symbolTable, &returnPvalue);
}

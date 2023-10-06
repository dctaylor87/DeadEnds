#include <stdio.h>
#include "standard.h"
#include "parse.h"
#include "interp.h"
#include "functiontable.h"
#include "pnode.h"
#include "errors.h"

//static bool debugging = false;

extern String currentProgramFileName;
extern int currentProgramLineNumber;
extern FunctionTable *procedureTable;

extern Database *simpleImportFromFile(FILE*, ErrorLog*);

int main()
{
	//  Create a database from the circle.ged file.
	FILE *gedcomFile = fopen("../Data/circle.ged", "r");
	ASSERT(gedcomFile);
	ErrorLog errorLog;
	Database *database = simpleImportFromFile(gedcomFile, &errorLog);
	ASSERT(database);
	printf("The number of persons in the database is %d.\n", numberPersons(database->personIndex));
	printf("The number of families in the database is %d.\n", numberFamilies(database->familyIndex));


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

//  test.c -- Test program.
//
//  Created by Thomas Wetmore on 5 October 2923.
//  Last changed on 7 October 2023.

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
extern Database *theDatabase;

extern Database *simpleImportFromFile(FILE*, ErrorLog*);

int main()
{
	//  Create a database from the circle.ged file.
	FILE *gedcomFile = fopen("../Data/circle.ged", "r");
	ASSERT(gedcomFile);
	ErrorLog errorLog;
	theDatabase = simpleImportFromFile(gedcomFile, &errorLog);
	ASSERT(theDatabase);
	printf("The number of persons in the database is %d.\n", numberPersons(theDatabase));
	printf("The number of families in the database is %d.\n", numberFamilies(theDatabase));

	// Show all the persons in the database.
	//  Iterate through the person index.

	showRecordIndex(theDatabase->personIndex);
	showRecordIndex(theDatabase->familyIndex);
	


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

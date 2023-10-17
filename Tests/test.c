//  test.c -- Test program.
//
//  Created by Thomas Wetmore on 5 October 2923.
//  Last changed on 16 October 2023.

#include <stdio.h>
#include "standard.h"
#include "parse.h"
#include "interp.h"
#include "functiontable.h"
#include "pnode.h"
#include "errors.h"
#include "sequence.h"

//static bool debugging = false;

extern String currentProgramFileName;
extern int currentProgramLineNumber;
extern FunctionTable *procedureTable;
extern Database *theDatabase;

extern Database *simpleImportFromFile(FILE*, ErrorLog*);
static void sequenceTests(FILE*);

int main()
{
	//  Create a database from the main.ged file.
	FILE *gedcomFile = fopen("../Gedfiles/main.ged", "r");
	FILE *outputFile = fopen("./Outputs/output.txt", "w");
	ASSERT(outputFile);
	//FILE *gedcomFile = fopen("../Gedfiles/circle.ged", "r");
	ASSERT(gedcomFile);
	ErrorLog errorLog;
	theDatabase = simpleImportFromFile(gedcomFile, &errorLog);
	ASSERT(theDatabase);
	printf("The number of persons in the database is %d.\n", numberPersons(theDatabase));
	printf("The number of families in the database is %d.\n", numberFamilies(theDatabase));
	sequenceTests(outputFile);

	// Show all the persons in the database.
	//  Iterate through the person index.

	//showRecordIndex(theDatabase->personIndex);
	//showRecordIndex(theDatabase->familyIndex);

	//  Parse a simple LifeLines "hello, world" program.
	parseProgram("llprogram", "../Reports");

	//  Create a PNProcCall node to call the main procedure with
	//currentProgramFileName = "internal";
	//currentProgramLineNumber = 1;
	//PNode *pnode = procCallPNode("main", null);

	//  Call the main procedure.
	//SymbolTable *symbolTable = createSymbolTable();
	//PValue returnPvalue;
	//interpret(pnode, symbolTable, &returnPvalue);
}

static int compare(Word a, Word b)
{
	String key1 = ((GNode*) a)->key;
	String key2 = ((GNode*) b)->key;
	ASSERT(key1 && key2);
	return compareRecordKeys(key1, key2);
}

void sequenceTests(FILE *outputFile)
{
	int i, j;
	printf("Hello, world!\n");
	//  Create a List of all the persons in the database.
	List *personList = createList(compare, null, null);
	GNode* person = firstInHashTable(theDatabase->personIndex, &i, &j);
	while (person) {
		appendListElement(personList, person);
		person = nextInHashTable(theDatabase->personIndex, &i, &j);
	}
	printf("The list has %d elements in it.\n", lengthList(personList));
	sortList(personList, true);
	FORLIST(personList, person)
		fprintf(outputFile, "%s\n", ((GNode*) person)->key);
	ENDLIST
}

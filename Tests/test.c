//  test.c -- Test program.
//
//  Created by Thomas Wetmore on 5 October 2923.
//  Last changed on 20 October 2023.

#include <stdio.h>
#include "standard.h"
#include "parse.h"
#include "interp.h"
#include "functiontable.h"
#include "recordindex.h"
#include "pnode.h"
#include "errors.h"
#include "sequence.h"

//static bool debugging = false;

static FILE *gedcomFile = null;
static FILE *outputFile = null;

extern String currentProgramFileName;
extern int currentProgramLineNumber;
extern FunctionTable *procedureTable;
extern Database *theDatabase;  // The database to use in the tests.

extern Database *simpleImportFromFile(FILE*, ErrorLog*);
static void createDatabaseTest(void);
static void listTest(FILE*);
static void forHashTableTest(void);
static void parseAndRunProgramTest(void);

extern bool validateDatabase(Database*);

int main()
{
	printf("createDatabaseTest\n");
	createDatabaseTest();
	printf("listTest\n"); fflush(stdout);
	listTest(outputFile);
	printf("forHashTableTest\n"); fflush(stdout);
	forHashTableTest();
	printf("parseAndRunProgramTest\n"); fflush(stdout);
	parseAndRunProgramTest();

	//validateDatabase(theDatabase);

	// Show all the persons in the database.
	//  Iterate through the person index.

	//showRecordIndex(theDatabase->personIndex);
	//showRecordIndex(theDatabase->familyIndex);

	
}

//  createDatabaseTest -- Creates a test database from a Gedcom file.
//-------------------------------------------------------------------------------------------------
void createDatabaseTest()
{
	//  Create a database from the main.ged file.
	gedcomFile = fopen("../Gedfiles/main.ged", "r");
	//gedcomFile = fopen("../Gedfiles/TWetmoreLine.ged", "r");
	outputFile = fopen("./Outputs/output.txt", "w");
	ErrorLog errorLog;
	theDatabase = simpleImportFromFile(gedcomFile, &errorLog);
	printf("The number of persons in the database is %d.\n", numberPersons(theDatabase));
	printf("The number of families in the database is %d.\n", numberFamilies(theDatabase));
}

//  compare -- Compare function required by the testList function following.
//-------------------------------------------------------------------------------------------------
static int compare(Word a, Word b)
{
	String key1 = ((GNode*) a)->key;
	String key2 = ((GNode*) b)->key;
	ASSERT(key1 && key2);
	return compareRecordKeys(key1, key2);
}

//  listTest -- Create a list of all the persons in the database, sort the list, and
//    print the tags of the records in the sorted order.
//-------------------------------------------------------------------------------------------------
void listTest(FILE *outputFile)
{
	int i, j;
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

void forHashTableTest(void)
{
	int numberValidated = 0;
	FORHASHTABLE(theDatabase->personIndex, element)
		numberValidated++;
		RecordIndexEl *rel = (RecordIndexEl*) element;
		GNode *person = rel->root;
		fprintf(outputFile, "%s: %s\n", person->key, NAME(person)->value);
	ENDHASHTABLE
	fprintf(outputFile, "%d persons were validated.\n", numberValidated);
}

//  parseAndRunProgramTest -- Parse a DeadEndScript program and run it. In order to call the
//    main procedure of a DeadEndScript, create a PNProcCall program node, and interpret it.
//-------------------------------------------------------------------------------------------------
void parseAndRunProgramTest(void)
{
	parseProgram("llprogram", "../Reports");
	//  Create a PNProcCall node to call the main procedure.
	currentProgramFileName = "internal";
	currentProgramLineNumber = 1;
	PNode *pnode = procCallPNode("main", null);

	//  Call the main procedure.
	SymbolTable *symbolTable = createSymbolTable();
	PValue returnPvalue;
	interpret(pnode, symbolTable, &returnPvalue);
}
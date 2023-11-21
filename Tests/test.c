//  test.c -- Test program.
//
//  Created by Thomas Wetmore on 5 October 2023.
//  Last changed on 16 November 2023.

#include <stdio.h>
#include "standard.h"
#include "parse.h"
#include "pnode.h"
#include "interp.h"
#include "functiontable.h"
#include "recordindex.h"
#include "errors.h"
#include "sequence.h"
#include "database.h"
#include "list.h"
#include "import.h"
#include "validate.h"

//static bool debugging = false;

#define VSCODE

static String gedcomFile = null;
static FILE *outputFile = null;

extern FunctionTable *procedureTable;

static Database *createDatabaseTest(void);
static void listTest(Database*, FILE*);
static void forHashTableTest(Database*);
static void parseAndRunProgramTest(Database*);
static void validateDatabaseTest(Database*);
static void forTraverseTest(Database*);

int main(void)
{
	printf("createDatabaseTest\n");
	Database *database = createDatabaseTest();
	printf("listTest\n");
	listTest(database, outputFile);
	printf("forHashTableTest\n");
	forHashTableTest(database);
	printf("parseAndRunProgramTest\n");
	//parseAndRunProgramTest();
	printf("indexNamesTest\n");
	indexNames(database);
	printf("validateDatabaseTest\n");
	validateDatabaseTest(database);
	printf("forTraverseTest\n");
	forTraverseTest(database);
	printf("parseAndRunProgramTest\n");
	parseAndRunProgramTest(database);

	return 0;
}

//  createDatabaseTest -- Creates a test database from a Gedcom file.
//-------------------------------------------------------------------------------------------------
Database *createDatabaseTest(void)
{
	//  Create a database from the main.ged file.
#ifdef XCODE
	gedcomFile = "/Users/ttw4/Desktop/DeadEndsCloneOne/CloneOne/CloneOne/Gedfiles/main.ged";
	outputFile = fopen("/Users/ttw4/Desktop/output.txt", "w");
#else
	gedcomFile = "../Gedfiles/main.ged";
	outputFile = fopen("./Outputs/output.txt", "w");
#endif
	ErrorLog *errorLog = createErrorLog();
	Database *database = importFromFile(gedcomFile, errorLog);
	printf("The number of persons in the database is %d.\n", numberPersons(database));
	printf("The number of families in the database is %d.\n", numberFamilies(database));
	return database;
}

//  compare -- Compare function required by the testList function that follows.
//-------------------------------------------------------------------------------------------------
static int compare(Word a, Word b)
{
	return compareRecordKeys(((GNode*) a)->key, ((GNode*) b)->key);
}

//  listTest -- Create a list of all the persons in the database, sort the list by tags, and
//    print the record tags in sorted order.
//-------------------------------------------------------------------------------------------------
void listTest(Database *database, FILE *outputFile)
{
	fprintf(outputFile, "\nStart of listTest\n");
	int i, j;  //  State variables used to iterate the person index hash table.
	GNode *person;
	//  Create a List of all the persons in the database.
	List *personList = createList(compare, null, null);
	Word element = firstInHashTable(database->personIndex, &i, &j);
	while (element) {
		person = ((RecordIndexEl*) element)->root;
		appendListElement(personList, person);
		element = nextInHashTable(database->personIndex, &i, &j);
	}
	printf("The list has %d elements in it.\n", lengthList(personList));
	sortList(personList, true);
	int count = 0;
	FORLIST(personList, person)
		fprintf(outputFile, "%s\n", ((GNode*) person)->key);
		count++;
	ENDLIST
	fprintf(outputFile, "%d persons are in the list\n", count);
	fprintf(outputFile, "End of listTest\n\n");
}

//  forHashTableTest -- Tests the FORHASHTABLE macro by showing all the persons in the database's
//    person index.
//-------------------------------------------------------------------------------------------------
void forHashTableTest(Database* database)
{
	fprintf(outputFile, "\nStart of FORHASHTABLE test\n");
	int numberPersons = 0;
	FORHASHTABLE(database->personIndex, element)
		numberPersons++;
		RecordIndexEl *rel = (RecordIndexEl*) element;
		GNode *person = rel->root;
		fprintf(outputFile, "%s: %s\n", person->key, NAME(person)->value);
	ENDHASHTABLE
	fprintf(outputFile, "%d persons were found in the index.\n", numberPersons);
	fprintf(outputFile, "End of FORHASHTABLE test\n\n");
}

//  parseAndRunProgramTest -- Parse a DeadEndScript program and run it. In order to call the
//    main procedure of a DeadEndScript, create a PNProcCall program node, and interpret it.
//-------------------------------------------------------------------------------------------------
void parseAndRunProgramTest(Database *database)
//  database -- The database the script runs on.
{
#ifdef VSCODE
	parseProgram("llprogram", "../Reports");
#else
	parseProgram("llprogram", "/Users/ttw4/Desktop/DeadEnds/Reports/");
#endif
//  Create a PNProcCall node to call the main procedure.
	currentProgramFileName = "internal";
	currentProgramLineNumber = 1;
	PNode *pnode = procCallPNode("main", null);

	//  Call the main procedure.
	SymbolTable *symbolTable = createSymbolTable();
	Context *context = createContext(symbolTable, database);
	PValue returnPvalue;
	interpret(pnode, context, &returnPvalue);
}

void validateDatabaseTest(Database *database)
{
	ErrorLog* errorLog = createErrorLog();
	validateDatabase(database, errorLog);
}

//  forTraverseTest -- Check that the FORTRAVERSE macro works.
//-------------------------------------------------------------------------------------------------
static void forTraverseTest(Database *database)
{
	GNode *person = keyToPerson("@I1@", database);
	FORTRAVERSE(person, node)
		printf("%s\n", node->tag);
	ENDTRAVERSE
}

//  test.c -- Test program.

//
//  Created by Thomas Wetmore on 5 October 2023.
//  Last changed on 11 November 2023.

#include <stdio.h>
#include "standard.h"
#include "parse.h"
#include "interp.h"
#include "functiontable.h"
#include "recordindex.h"
#include "pnode.h"
#include "errors.h"
#include "sequence.h"
#include "database.h"
#include "list.h"

//static bool debugging = false;

static FILE *gedcomFile = null;
static FILE *outputFile = null;

extern FunctionTable *procedureTable;
extern Database *theDatabase;  // The database to use in the tests.

extern Database *importFromFile(String, ErrorLog*);
static void createDatabaseTest(void);
static void listTest(FILE*);
static void forHashTableTest(void);
static void parseAndRunProgramTest(void);
static void validateDatabaseTest(void);
static void forTraverseTest(void);

extern bool validateDatabase(Database*, ErrorLog*);

int main(void)
{
	printf("createDatabaseTest\n");
	createDatabaseTest();
	printf("listTest\n");
	listTest(outputFile);
	printf("forHashTableTest\n");
	forHashTableTest();
	printf("parseAndRunProgramTest\n");
	parseAndRunProgramTest();
	printf("indexNamesTest\n");
	indexNames(theDatabase);
	printf("validateDatabaseTest\n");
	validateDatabaseTest();
	printf("forTraverseTest\n");
	forTraverseTest();

	//showRecordIndex(theDatabase->personIndex);
	//showRecordIndex(theDatabase->familyIndex);
	return 0;
}

//  createDatabaseTest -- Creates a test database from a Gedcom file.
//-------------------------------------------------------------------------------------------------
void createDatabaseTest(void)
{
	//  Create a database from the main.ged file.
	gedcomFile = fopen("../Gedfiles/main.ged", "r");
	//gedcomFile = fopen("/Users/ttw4/Desktop/DeadEndsCloneOne/CloneOne/CloneOne/Gedfiles/main.ged", "r");
	//gedcomFile = fopen("../Gedfiles/TWetmoreLine.ged", "r");
	outputFile = fopen("./Outputs/output.txt", "w");
	//outputFile = fopen("/Users/ttw4/Desktop/output.txt", "w");
	ErrorLog *errorLog = createErrorLog();
	theDatabase = importFromFile("../Gedfiles/main.ged", errorLog);
	printf("The number of persons in the database is %d.\n", numberPersons(theDatabase));
	printf("The number of families in the database is %d.\n", numberFamilies(theDatabase));
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
void listTest(FILE *outputFile)
{
	fprintf(outputFile, "\nStart of listTest\n");
	int i, j;  //  State variables used to iterate the person index hash table.
	GNode *person;
	//  Create a List of all the persons in the database.
	List *personList = createList(compare, null, null);
	Word element = firstInHashTable(theDatabase->personIndex, &i, &j);
	while (element) {
		person = ((RecordIndexEl*) element)->root;
		appendListElement(personList, person);
		element = nextInHashTable(theDatabase->personIndex, &i, &j);
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
void forHashTableTest(void)
{
	fprintf(outputFile, "\nStart of FORHASHTABLE test\n");
	int numberPersons = 0;
	FORHASHTABLE(theDatabase->personIndex, element)
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

void validateDatabaseTest(void)
{
	ErrorLog* errorLog = createErrorLog();
	validateDatabase(theDatabase, errorLog);
}

//  forTraverseTest -- Check that the FORTRAVERSE macro works.
//-------------------------------------------------------------------------------------------------
static void forTraverseTest(void)
{
	GNode *person = keyToPerson("@I1@", theDatabase);
	FORTRAVERSE(person, node)
		printf("%s\n", node->tag);
	ENDTRAVERSE
}

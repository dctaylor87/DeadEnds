//  test.c -- Test program.
//
//  Created by Thomas Wetmore on 5 October 2023.
//  Last changed on 10 December 2023.

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
#include "path.h"
#include "readnode.h"

extern FunctionTable *procedureTable;

static Database *createDatabaseTest(String, int, ErrorLog*);
static void listTest(Database*, int);
static void forHashTableTest(Database*, int);
static void parseAndRunProgramTest(Database*, int);
static void validateDatabaseTest(Database*, int);
static void forTraverseTest(Database*, int);
static void showHashTableTest(HashTable*, int);
static void indexNamesTest(Database*, int);
static void testNewReadLayer(String);
extern bool validateDatabase(Database*, ErrorLog*);

//  main -- Main function for the batch program that tests the DeadEnds infrastructure software.
//-------------------------------------------------------------------------------------------------
int main (void)
{
	//String gedcomFile = "/Users/ttw4/Desktop/DeadEnds/Gedfiles/bad.ged";
	String gedcomFile = "/Users/ttw4/Desktop/DeadEnds/Gedfiles/main.ged";
	//String gedcomFile = "/Users/ttw4/Desktop/DeadEnds/Gedfiles/small.ged";
	//String gedcomFile = "/Users/ttw4/Desktop/DeadEnds/Gedfiles/notthere.ged";
	//String gedcomFile = "/Users/ttw4/Desktop/DeadEnds/Gedfiles/threezeros.ged";
	int testNumber = 0;
	ErrorLog *errorLog = createErrorLog();

	// OKAY, got to stop this stuff and get a new read laryer working. So ...
	testNewReadLayer(gedcomFile);
	return 0;

	Database *database = createDatabaseTest(gedcomFile, ++testNumber, errorLog);
	showErrorLog(errorLog);
	//return 0;  // EXPEDIENT.

	if (database) listTest(database, ++testNumber);

	if (database) forHashTableTest(database, ++testNumber);

	if (database) showHashTableTest(database->personIndex, ++testNumber);

	if (database) indexNamesTest(database, ++testNumber);

	if (database) validateDatabaseTest(database, ++testNumber);

	if (database) forTraverseTest(database, ++testNumber);

	if (database) parseAndRunProgramTest(database, ++testNumber);

	return 0;
}

//  createDatabaseTest -- Creates a test database from a Gedcom file.
//-------------------------------------------------------------------------------------------------
Database *createDatabaseTest(String gedcomFile, int testNumber, ErrorLog *errorLog)
{
	printf("%d: START OF CREATE DATABASE TEST -- Create database from %s\n", testNumber, gedcomFile);
	String lastSegment = lastPathSegment(gedcomFile);
	printf("lastPathSegment: %s\n", lastSegment);
	Database *database = importFromFile(gedcomFile, errorLog);
	printf("The number of persons in the database is %d.\n", numberPersons(database));
	printf("The number of families in the database is %d.\n", numberFamilies(database));
	printf("END OF CREATE DATABASE TEST\n");
	return database;
}

//  compare -- Compare function needed by the listTest function that follows.
//-------------------------------------------------------------------------------------------------
static int compare(Word a, Word b)
{
	return compareRecordKeys(((GNode*) a)->key, ((GNode*) b)->key);
}

//  listTest -- Create a list of the persons in the database; sort the list by tags, and print
//    the record tags in sorted order.
//-------------------------------------------------------------------------------------------------
void listTest(Database *database, int testNumber)
{
	printf("\n%d: START OF LIST TEST\n", testNumber);
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
		printf("%s\n", ((GNode*) person)->key);
		count++;
	ENDLIST
	printf("%d persons are in the list\n", count);
	printf("END OF LIST TEST\n");
}

//  forHashTableTest -- Tests the FORHASHTABLE macro by showing all the persons in the database's
//    person index.
//-------------------------------------------------------------------------------------------------
void forHashTableTest(Database* database, int testNumber)
{
	printf("\n%d: START OF FORHASHTABLE test\n", testNumber);
	int numberPersons = 0;
	FORHASHTABLE(database->personIndex, element)
		numberPersons++;
		RecordIndexEl *rel = (RecordIndexEl*) element;
		GNode *person = rel->root;
		printf("%s: %s\n", person->key, NAME(person)->value);
	ENDHASHTABLE
	printf("%d persons were found in the index.\n", numberPersons);
	printf("END OF FORHASHTABLE TEST\n");
}

//  parseAndRunProgramTest -- Parse a DeadEndScript program and run it. In order to call the
//    main procedure of a DeadEndScript, create a PNProcCall program node, and interpret it.
//-------------------------------------------------------------------------------------------------
void parseAndRunProgramTest(Database *database, int testNumber)
//  database -- The database the script runs on.
{
	printf("\n%d: START OF PARSE AND RUN PROGRAM TEST\n", testNumber);

//  Create a PNProcCall node to call the main procedure.
	currentProgramFileName = "internal";
	currentProgramLineNumber = 1;
	PNode *pnode = procCallPNode("main", null);

	//  Call the main procedure.
	SymbolTable *symbolTable = createSymbolTable();
	Context *context = createContext(symbolTable, database);
	PValue returnPvalue;
	interpret(pnode, context, &returnPvalue);
	printf("END OF PARSE AND RUN PROGRAM TEST\n");
}

//  validateDatabaseTest -- Validate the a database.
//-------------------------------------------------------------------------------------------------
void validateDatabaseTest(Database *database, int testNumber)
{
	printf("\n%d: START OF VALIDATE DATABASE TEST\n", testNumber);
	ErrorLog* errorLog = createErrorLog();
	validateDatabase(database, errorLog);
	printf("END OF VALIDATE DATABASE TEST\n");
}

//  forTraverseTest -- Check that the FORTRAVERSE macro works.
//-------------------------------------------------------------------------------------------------
static void forTraverseTest(Database *database, int testNumber)
{
	printf("\n%d: START OF FORTRAVERSE TEST\n", testNumber);
	GNode *person = keyToPerson("@I1@", database);
	FORTRAVERSE(person, node)
		printf("%s\n", node->tag);
	ENDTRAVERSE
	printf("END OF FORTRAVERSE TEST\n");
}

static void showPersonName(Word element)
{
	GNode *person = ((RecordIndexEl*) element)->root;
	ASSERT(person);
	GNode *name = NAME(person);
	ASSERT(name && person->key && name->value);
	printf("%s: %s", person->key, name->value);
}

static void showHashTableTest(RecordIndex *index, int testNumber)
{
	printf("\n%d: START OF SHOW HASH TABLE TEST\n", testNumber);
	showHashTable(index, showPersonName);
	printf("END OF SHOW HASH TABLE TEST\n");
}

static void indexNamesTest(Database *database, int testNumber)
{
	printf("\n%d: START OF INDEX NAMES TEST\n", testNumber);
	indexNames(database);
	printf("END OF INDEX NAMES TEST\n");
}

NodeList *upperHalf(NodeList*, ErrorLog*);
static void testNewReadLayer(String fileName)
{
	printf("Hello from testNewReadLayer\n");
	FILE *fp = fopen(fileName, "r");
	if (fp == NULL) {
		printf("FAILURE IN TESTNEWREADLAYER\n");
		return;
	}
	ErrorLog *errorLog = createErrorLog();
	
	// getNodeList is the "lower half" of the new read stack layer.
	NodeList *nodeList = getNodeList(fp, errorLog);
	//showNodeList(nodeList);
	//  upperHalf is the "upper half" of the new read stack layer. Need better names.
	NodeList *rootNodeList = upperHalf(nodeList, errorLog);
	printf("The length of the rootNodeList is %d\n", lengthList(rootNodeList));
	FORLIST(rootNodeList, element)
		NodeListElement *e = (NodeListElement*) element;
		showGNode(e->node);
	ENDLIST
}

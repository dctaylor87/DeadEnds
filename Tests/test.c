//  test.c -- Test program.
//
//  Created by Thomas Wetmore on 5 October 2023.
//  Last changed on 22 November 2023.

#include <stdio.h>
#include "standard.h"
#include "parse.h"
#include "interp.h"
#include "functiontable.h"
#include "recordindex.h"
#include "pnode.h"
#include "errors.h"
#include "sequence.h"
#include "list.h"
#include "path.h"

#define VSCODE

static String gedcomFile = null;
static FILE *outputFile = null;

extern String currentProgramFileName;
extern int currentProgramLineNumber;
extern FunctionTable *procedureTable;

extern Database *importFromFile(String, ErrorLog*);
static Database *createDatabaseTest(void);
static void listTest(Database*, FILE*);
static void forHashTableTest(Database*);
static void parseAndRunProgramTest(Database*);
static void validateDatabaseTest(Database*);
static void forTraverseTest(Database*);
static void showHashTableTest(HashTable*);
static void indexNamesTest(Database *database);
extern bool validateDatabase(Database*, ErrorLog*);

int main (void)
{
	Database *database = createDatabaseTest();

	listTest(database, outputFile);

	forHashTableTest(database);

	showHashTableTest(database->personIndex);

	indexNamesTest(database);

	validateDatabaseTest(database);

	forTraverseTest(database);

	parseAndRunProgramTest(database);

	return 0;
}

//  createDatabaseTest -- Creates a test database from a Gedcom file.
//-------------------------------------------------------------------------------------------------
Database *createDatabaseTest(void)
{
	printf("START OF CREATE DATABASE TEST -- Create database from main.ged\n");
#ifdef XCODE
	gedcomFile = "/Users/ttw4/Desktop/DeadEndsCloneOne/CloneOne/CloneOne/Gedfiles/main.ged";
	outputFile = fopen("/Users/ttw4/Desktop/output.txt", "w");
#else
	gedcomFile = "../Gedfiles/main.ged";
	outputFile = fopen("./Outputs/output.txt", "w");
#endif
	ErrorLog *errorLog = createErrorLog();
	//printf("gedcomFile: %s\n", gedcomFile);
	String lastSegment = lastPathSegment(gedcomFile);
	printf("lastPathSegment: %s\n", lastSegment);
	Database *database = importFromFile(gedcomFile, errorLog);
	printf("The number of persons in the database is %d.\n", numberPersons(database));
	printf("The number of families in the database is %d.\n", numberFamilies(database));
	printf("END OF CREATE DATABASE TEST\n");
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
	fprintf(outputFile, "\nSTART OF LIST TEST\n");
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
	fprintf(outputFile, "END OF LIST TEST\n\n");
}

//  forHashTableTest -- Tests the FORHASHTABLE macro by showing all the persons in the database's
//    person index.
//-------------------------------------------------------------------------------------------------
void forHashTableTest(Database* database)
{
	fprintf(outputFile, "\nSTART OF FORHASHTABLE test\n");
	int numberPersons = 0;
	FORHASHTABLE(database->personIndex, element)
		numberPersons++;
		RecordIndexEl *rel = (RecordIndexEl*) element;
		GNode *person = rel->root;
		fprintf(outputFile, "%s: %s\n", person->key, NAME(person)->value);
	ENDHASHTABLE
	fprintf(outputFile, "%d persons were found in the index.\n", numberPersons);
	fprintf(outputFile, "END OF FORHASHTABLE TEST\n\n");
}

//  parseAndRunProgramTest -- Parse a DeadEndScript program and run it. In order to call the
//    main procedure of a DeadEndScript, create a PNProcCall program node, and interpret it.
//-------------------------------------------------------------------------------------------------
void parseAndRunProgramTest(Database *database)
//  database -- The database the script runs on.
{
	printf("START OF PARSE AND RUN PROGRAM TEST\n");
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
	printf("END OF PARSE AND RUN PROGRAM TEST\n");
}

//  validateDatabaseTest -- Validate the a database.
//-------------------------------------------------------------------------------------------------
void validateDatabaseTest(Database *database)
{
	printf("START OF VALIDATE DATABASE TEST\n");
	ErrorLog* errorLog = createErrorLog();
	validateDatabase(database, errorLog);
	printf("END OF VALIDATE DATABASE TEST\n");
}

//  forTraverseTest -- Check that the FORTRAVERSE macro works.
//-------------------------------------------------------------------------------------------------
static void forTraverseTest(Database *database)
{
	printf("START OF FORTRAVERSE TEST\n");
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

static void showHashTableTest(RecordIndex *index)
{
	printf("START OF SHOW HASH TABLE TEST\n");
	showHashTable(index, showPersonName);
	printf("END OF SHOW HASH TABLE TEST\n");
}

static void indexNamesTest(Database *database)
{
	printf("START OF INDEX NAMES TEST\n");
	indexNames(database);
	printf("END OF INDEX NAMES TEST\n");
}

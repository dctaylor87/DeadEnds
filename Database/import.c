//
//  DeadEnds
//
//  import.c -- Read Gedcom files and build a database from them.
//
//  Created by Thomas Wetmore on 13 November 2022.
//  Last changed on 19 November 2023.
//

#include "standard.h"
#include "import.h"
#include "gnode.h"
#include "stringtable.h"
#include "recordindex.h"
#include "gedcom.h"
#include "splitjoin.h"
#include "database.h"
#include "validate.h"
#include "errors.h"
#include "readnode.h"

String currentGedcomFileName = null;
int currentGedcomLineNumber = 1;

extern bool validateIndex(RecordIndex *index);
static String updateKeyMap(GNode *root, StringTable* keyMap);
static void rekeyIndex(RecordIndex*, StringTable *keyMap);
static void outputErrorLog(ErrorLog* errorLog);
static void setupDatabase(List *recordIndexes);
static void addIndexToDatabase(RecordIndex *index, Database *database);

Database *importFromFile(String, ErrorLog*);

// Error messages defined elsewhere.
extern String idgedf, gdcker, gdnadd, dboldk, dbnewk, dbodel, cfoldk, dbdelk, dbrdon;

static GNode *normalizeNodeTree (GNode*);
static bool debugging = true;

//  importFromFiles -- Import Gedcom files into a list of Databases.
//--------------------------------------------------------------------------------------------------
List *importFromFiles(String fileNames[], int count, ErrorLog *errorLog)
//  filesNames -- Names of the files to import.
//  count -- Number of files to import.
//  errorLog -- Error log.
{
	List *listOfDatabases = createList(null, null, null);
	Database *database = null;

	for (int i = 0; i < count; i++) {
		if ((database = importFromFile(fileNames[i], errorLog)))
			appendListElement(listOfDatabases, database);
	}
	return listOfDatabases;
}

//  importFromFile -- Import the records in a Gedcom file into a Database.
//--------------------------------------------------------------------------------------------------
Database *importFromFile(String fileName, ErrorLog *errorLog)
{
	if (debugging) printf("Entered importFromFile\n");
	ASSERT(fileName);
	FILE *file = fopen(fileName, "r");
	if (!file) {
		addErrorToLog(errorLog, createError(systemError, fileName, 0, "could not open file"));
		return null;
	}

	Database *database = createDatabase(fileName);
	int recordCount = 0;  // DEBUG: Remove after testing.
	int lineNo;

	//  Read the records and add them to the database.
	GNode *root = firstNodeTreeFromFile(file, &lineNo, errorLog);
	while (root) {
		recordCount++;  // DEBUG: Remove after testing.
		storeRecord(database, normalizeNodeTree(root), lineNo);
		root = nextNodeTreeFromFile(file, &lineNo, errorLog);
	}
	if (debugging) printf("Read %d records.\n", recordCount);

	return database;
}

String misnam = (String) "Missing NAME line in INDI record; record ignored.\n";
String noiref = (String) "FAM record has no INDI references; record ignored.\n";

// normalizeNodeTree -- Normalize node tree records to standard format.
//--------------------------------------------------------------------------------------------------
static GNode *normalizeNodeTree(GNode *root)
//  root -- Root of a gedcom node tree record.
{
	//  Don't worry about HEAD or TRLR records.
	if (eqstr("HEAD", root->tag) || eqstr("TRLR", root->tag)) return root;

	//  Normalize the node tree records to standard format.
	switch (recordType(root)) {
		case GRPerson: return normalizePerson(root);
		case GRFamily:  return normalizeFamily(root);
		case GREvent: return normalizeEvent(root);
		case GRSource: return normalizeSource(root);
		case GROther: return normalizeOther(root);
		default: FATAL();
	}
	return null;
}

//  outputErrorLog
//--------------------------------------------------------------------------------------------------
static void outputErrorLog(ErrorLog* errorLog)
{
	sortList(errorLog, true);
	for (int index = 0; index < lengthList(errorLog); index++) {
		Error *error = getListElement(errorLog, index);
		printf("Error in file: %s at line %d: %s\n", error->fileName, error->lineNumber,
			   error->message);
	}
}                                                                                      

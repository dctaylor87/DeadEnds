//
//  DeadEnds
//
//  import.c -- Read Gedcom files and build a database from them.
//
//  Created by Thomas Wetmore on 13 November 2022.
//  Last changed on 7 November 2023.
//

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */

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

Database *theDatabase = null;

extern bool validateIndex(RecordIndex *index);
static String updateKeyMap(GNode *root, StringTable* keyMap);
static void rekeyDatabase(Database*, StringTable *keyMap);
static void rekeyIndex(RecordIndex*, StringTable *keyMap);
static void outputErrorLog(ErrorLog* errorLog);
static void setupDatabase(List *recordIndexes);
static void addIndexToDatabase(RecordIndex *index, Database *database);

// Error messages defined elsewhere.
extern String idgedf, gdcker, gdnadd, dboldk, dbnewk, dbodel, cfoldk, dbdelk, dbrdon;

static GNode *normalizeNodeTree (GNode*);
static bool debugging = true;

//  importFromFiles -- Import Gedcom files into a list of Databases. This function not yet used.
//--------------------------------------------------------------------------------------------------
bool importFromFiles(String fileNames[], int count, ErrorLog *errorLog)
//  filesNames -- Names of the files to import.
//  count -- Number of files to import.
//  errorLog -- Error log.
{
	//  Create a list Databases, one for each Gedcom file that is read successfully.
	List *listOfDatabases = createList(null, null, null);
	unused(listOfDatabases);

	//  Loop through the file names importing each into a Database.
	for (int i = 0; i < count; i++) {
		Database *database = importFromFile(fileNames[i], errorLog);
		if (!database) {
			if (debugging) {
				printf("There was an error importing from file %s.\n", fileNames[i]);
				printf("There should be errors in the log that say what's wrong.\n");
			}
			continue;
		}
	}
	return true;
}

//  importFromFile -- Import the records from a Gedcom file into an index. After reading the
//    records rekey the records.
//--------------------------------------------------------------------------------------------------
Database *importFromFile(CString fileName, ErrorLog *errorLog)
{
	if (debugging) printf("Entered simpleImportFromFile\n");
	ASSERT(fileName);
	FILE *file = fopen(fileName, "r");
	if (!file) {
		addErrorToLog(errorLog, systemError, fileName, 0, "could not open file.");
		return null;
	}

	return importFromFileFP (file, fileName, errorLog);
}

Database *importFromFileFP (FILE *file, CString fileName, ErrorLog *errorLog)
{
	Database *database = createDatabase(fileName);

	String errorMessage;
	int recordCount = 0;

	//  Read the records and add them to the database.
	GNode *root = firstNodeTreeFromFile(file, &errorMessage);
	while (root) {
		recordCount++;
		storeRecord(database, root);
		root = nextNodeTreeFromFile(file, &errorMessage);
	}
	printf("Read %d records.\n", recordCount);

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
	if (eqstr("HEAD", root->tag) || eqstr("TRLR", root->tag)) return null;

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

// DeadEnds
//
// import.c has functions that import Gedcom files into internal structures.
//
// Created by Thomas Wetmore on 13 November 2022.
// Last changed on 27 May 2024.

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include <unistd.h>
#include <errno.h>
#include <sys/param.h>
#include <sys/errno.h>
#include <stdlib.h>

#include "standard.h"
#include "refnindex.h"
#include "file.h"
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
#include "gnodelist.h"
#include "path.h"

extern FILE* debugFile;

// Local flags.
static bool debugging = true;
bool importDebugging = true;

// toString returns the GNode in a GNodeListElement as a string; for debugging.
static String toString(void* element) {
	GNode* gnode = ((GNodeListEl*) element)->node;
	return gnodeToString(gnode, 0);
}

// importFromFiles imports a list of Gedcom files into a List of Databases, one per file. If errors
// are found in a file the file's Database is not created and the ErrorLog will hold the errors.
List* importFromFiles(String filePaths[], int count, ErrorLog* errorLog) {
	List* listOfDatabases = createList(null, null, null, false);
	Database* database = null;
	for (int i = 0; i < count; i++) {
		if ((database = importFromFile(filePaths[i], errorLog)))
			appendToList(listOfDatabases, database);
	}
	return listOfDatabases;
}

// importFromFile imports the records in a Gedcom file into a new Database. If errors are found
// the function returns null, and ErrorLog holds the Errors.
Database *importFromFile(CString filePath, ErrorLog* errorLog) {
	if (importDebugging) printf("IMPORT FROM FILE: start: %s\n", filePath);
	// Open the Gedcom file.
	File* file = createFile(filePath, "r");
	if (!file) {
		addErrorToLog(errorLog, createError(systemError, filePath, 0, "Could not open file."));
		return null;
	}

	return importFromFileFP (file, filePath, errorLog);
}

Database *importFromFileFP (File *file, CString filePath, ErrorLog *errorLog)
{
	String lastSegment = lastPathSegment(filePath); // MNOTE: strsave not needed.

	if (importDebugging)
		fprintf(debugFile, "importFromFile: calling getNodeListFromFile(%s,...\n", filePath);
	int numErrors = 0;
	GNodeList* listOfNodes = getGNodeListFromFile(file, errorLog); // Get all lines as GNodes.
	if (!listOfNodes) return null;

#if defined(DEADENDS)
	if (importDebugging)
		fprintf(debugFile, "return from getNodeListFromFile: numErrors = %d\n", numErrors);
	if (numErrors > 0)
		return null;	// import failed -- errors found
#endif
	if (importDebugging) {
		fprintf(debugFile, "importFromFile: back from getNodeListFromFile\n");
		fprintf(debugFile, "importFromFile: listOfNodes contains\n");
		fprintfBlock(debugFile, &(listOfNodes->block), toString);
	}
	// Convert the NodeList of GNodes and Errors into a GNodeList of GNode trees.
	if (importDebugging) fprintf(debugFile, "importFromFile: calling getNodeTreesFromNodeList\n");
	GNodeList* listOfTrees = getNodeTreesFromNodeList(listOfNodes, file->name, errorLog);
	if (importDebugging) fprintf(debugFile, "importFromFile: back from getNodeTreesFromNodeList\n");
	if (importDebugging) {
		fprintf(debugFile, "importFromFile: listOfGTrees contains\n");
		//fprintfBlock(debugFile, &(listOfTrees->block), toString);
	}
#if !defined(DEADENDS)
	if (numErrors) { // Temporary early exit.
		printf("There are %d errors in the listOfTrees. Bailing for the time being.\n", numErrors);
		exit(1);
	}
#endif
	Database* database = createDatabase(filePath); // Create database and add records to it.
	FORLIST(listOfTrees, element)
		GNodeListEl* e = (GNodeListEl*) element;
		storeRecord(database, normalizeRecord(e->node), e->line, errorLog);
	ENDLIST
	printf("And now it is time to sort those RootList\n");
	sortList(database->personRoots);
	printf("Persons have been sorted\n");
	sortList(database->familyRoots);
	printf("Families have been sorted\n");

	if (debugging) {
		printf("There were %d gnode tree records extracted from the file.\n", lengthList(listOfTrees));
		printf("There were %d errors importing file %s.\n", lengthList(errorLog), file->name);
		showErrorLog(errorLog);
	}
	if (lengthList(errorLog) > 0) {
		deleteDatabase(database);
		return null;
	}
	return database;
}

String misnam = (String) "Missing NAME line in INDI record; record ignored.\n";
String noiref = (String) "FAM record has no INDI references; record ignored.\n";


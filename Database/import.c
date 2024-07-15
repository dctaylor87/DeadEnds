// DeadEnds
//
// import.c has functions that import Gedcom files into internal structures.
//
// Created by Thomas Wetmore on 13 November 2022.
// Last changed on 13 July 2024.

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
#include "utils.h"
#include "stringset.h"

extern FILE* debugFile;

// Local flags.
static bool timing = true; // Prints time at milestones.
//static bool debugging = true;
bool importDebugging = true; // Detail debugging of import.

static void checkKeysAndReferences(GNodeList*, String name, ErrorLog*);

// toString returns the GNode in a GNodeListEl as a string; for debugging.
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
		if ((database = gedcomFileToDatabase(filePaths[i], errorLog)))
			appendToList(listOfDatabases, database);
	}
	return listOfDatabases;
}

// gedcomFileToDatabase returns the Database of a single Gedcom file. Returns null if no Database
// is created. errorLog holds any Errors found.
Database* gedcomFileToDatabase(CString path, ErrorLog* log) {
	if (timing) printf("Start of gedcomFileToDatabase: %s.\n", getMillisecondsString());
	// Open the Gedcom file.
	File* file = createFile(path, "r");
	if (!file) {
		addErrorToLog(log, createError(systemError, path, 0, "Could not open file."));
		return null;
	}

	return importFromFileFP (file, path, log);
}

Database *importFromFileFP (File *file, CString path, ErrorLog *log)
{
	String lastSegment = lastPathSegment(path); // MNOTE: strsave not needed.

	// Get the list of GNode roots from file. These are all records from the Gedcom file including
	// HEAD and TRLR. If errors occur rootList will be null and the error log will hold Errors.
	GNodeList* rootList = getGNodeTreesFromFile(file, log);
	deleteFile(file);
	if (timing) printf("Got rootList: %s.\n", getMillisecondsString());
	if (importDebugging) printf("rootList contains %d records.\n", lengthList(rootList));
	if (lengthList(log)) {
		deleteGNodeList(rootList, true);
		return null;
	}
	// Check all keys and their references. Duplicate keys are not allowed and all references
	// must be to existing keys.
	checkKeysAndReferences(rootList, file->name, log);
	if (timing) printf("Checked keys: %s.\n", getMillisecondsString());
	if (lengthList(log)) {
		deleteGNodeList(rootList, true);
		return null;
	}

	// Create personIndex for INDI records, familyIndex for FAM records, and recordIndex for
	// all keyed records.
	RecordIndex* personIndex = createRecordIndex();
	RecordIndex* familyIndex = createRecordIndex();
	RecordIndex* recordIndex = createRecordIndex();
	int otherRecords = 0;
	FORLIST(rootList, element)
		GNodeListEl* el = (GNodeListEl*) element;
		GNode* root = el->node;
		if (root->key)
			addToRecordIndex(recordIndex, root->key, root, el->line);
		if (recordType(root) == GRPerson)
			addToRecordIndex(personIndex, root->key, root, el->line);
		else if (recordType(root) == GRFamily)
			addToRecordIndex(familyIndex, root->key, root, el->line);
		else
			otherRecords++;
	ENDLIST
	deleteGNodeList(rootList, false);
	if (timing) printf("Created the three indexes: %s.\n", getMillisecondsString());
	if (importDebugging) {
		printf("The person index holds %d records.\n", sizeHashTable(personIndex));
		printf("The family index holds %d records.\n", sizeHashTable(familyIndex));
		printf("The record index holds %d records.\n", sizeHashTable(recordIndex));
		printf("There are %d other records.\n", otherRecords);
	}
	// Create the Database and add the indexes.
	Database* database = createDatabase(path);
	database->recordIndex = recordIndex;
	database->personIndex = personIndex;
	database->familyIndex = familyIndex;
	// Validate persons and families.
	validatePersons(database, log);
	if (timing) printf("Validated persons: %s.\n", getMillisecondsString());
	validateFamilies(database, log);
	if (timing) printf("Validated families: %s.\n", getMillisecondsString());
	if (lengthList(log)) {
		deleteDatabase(database);
		return null;
	}
	// Create the name index.
	getNameIndexForDatabase(database);
	if (timing) printf("Indexed names: %s.\n", getMillisecondsString());
	// Create the REFN index and validate it.
	validateReferences(database, log);
	if (timing) printf("Indexed REFNs: %s.\n", getMillisecondsString());
	if (timing) printf("End of gedcomFileToDatabase: %s.\n", getMillisecondsString());
	if (lengthList(log)) {
		deleteDatabase(database);
		return null;
	}
	return database;
}

// checkKeysAndReferences checks record keys and their references. Creates a table of all keys
// and checks for duplicates. CHecks that all keys found as values refer to records.
static void checkKeysAndReferences(GNodeList* records, String name, ErrorLog* log) {
	StringSet* keySet = createStringSet();
	FORLIST(records, element)
		GNodeListEl* el = (GNodeListEl*) element;
		GNode* root = el->node;
		String key = root->key;
		if (!key) {
			RecordType rtype = recordType(root);
			if (rtype == GRHeader || rtype == GRTrailer) continue;
			addErrorToLog(log, createError(gedcomError, name, el->line, "record missing a key"));
			continue;
		}
		if (isInSet(keySet, key)) {
			addErrorToLog(log, createError(gedcomError, name, el->line, "duplicate key"));
			continue;
		}
		addToSet(keySet, key);
	ENDLIST
	// Check that keys used as values are in the set.
	int numReferences = 0; // Debug and sanity.
	int nodesTraversed = 0; // Debug and sanity.
	int recordsVisited = 0; // Debug and sanity.
	int nodesCounted = 0; // Debug and sanity.
	FORLIST(records, element)
		recordsVisited++;
		GNodeListEl* el = (GNodeListEl*) element;
		GNode* root = el->node;
		nodesCounted += countNodes(root);
		FORTRAVERSE(root, node)
			nodesTraversed++;
			if (isKey(node->value)) numReferences++;
			if (isKey(node->value) && !isInSet(keySet, node->value)) {
				Error* error = createError(
								gedcomError,
								name,
								el->line + countNodesBefore(node),
								"invalid key value");
					addErrorToLog(log, error);
					printf("Didn't find key: %s\n", node->value);
				}
		ENDTRAVERSE
	ENDLIST
	if (importDebugging) {
		printf("The length of the key set is %d.\n", lengthSet(keySet));
		printf("The length of the error log is %d.\n", lengthList(log));
		printf("The number of references to keys is %d.\n", numReferences);
		printf("The number of records visited is %d.\n", recordsVisited);
		printf("The number of nodes traversed is %d.\n", nodesTraversed);
		printf("The number of nodes counted is %d.\n", nodesCounted);
	}
}

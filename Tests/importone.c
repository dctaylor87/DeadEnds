// DeadEnds
//
// importone.c has a test program for importing a single Gedcom file into a Database.
//
// Created by Thomas Wetmore on 21 June 2024.
// Last changed on 5 July 2024.

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "standard.h"
#include "errors.h"

//#include <unistd.h>
#include <errno.h>
//#include <sys/param.h>
//#include <sys/errno.h>
//#include <stdlib.h>
//#include "import.h"
//#include "gnode.h"
//#include "stringtable.h"
//#include "recordindex.h"
//#include "gedcom.h"
#include "splitjoin.h"
//#include "database.h"
#include "validate.h"
//#include "readnode.h"
#include "stringset.h"
#include "gnodelist.h"
#include "path.h"

static bool importDebugging = true;

void checkKeysAndReferences(GNodeList*, String name, ErrorLog*);
Database* gedcomFileToDatabase(String, ErrorLog*);

int importTest(void) {
	//String gedcomFile = "/Users/ttw4/Desktop/DeadEnds/Gedfiles/051224.ged";
	String gedcomFile = "/Users/ttw4/Desktop/DeadEnds/Gedfiles/07022024.ged";
	//String gedcomFile = "/Users/ttw4/Desktop/DeadEnds/Gedfiles/smallish.ged";
	//String gedcomFile = "/Users/ttw4/Desktop/DeadEnds/Gedfiles/parenterror.ged";
	ErrorLog* errorLog = createErrorLog();
	String lastSegment = lastPathSegment(gedcomFile);
	printf("lastPathSegment: %s\n", lastSegment);
	Database* database = gedcomFileToDatabase(gedcomFile, errorLog);
	if (lengthList(errorLog)) {
		printf("Import cancelled because of errors:\n");
		showErrorLog(errorLog);
	}
	return 0;
}

// FOLLOWING FUNCTION WILL BE MOVED TO ITS PROPER PLACE LATER.

// gedcomFileToDatabase returns the Database of a single Gedcom file. Returns null if no Database
// is created; the errorLog holds any errors encountered while processing the file.
Database* gedcomFileToDatabase(String filePath, ErrorLog* errorLog) {
	// Open Gedcom file.
	File* file = createFile(filePath, "r");
	if (!file) {
		addErrorToLog(errorLog, createError(systemError, filePath, 0, "Could not open file."));
		return null;
	}
	// Get list of GNodes from file.
	GNodeList* listOfNodes = getNodeListFromFile(file, errorLog);
	if (!listOfNodes) return null;
	if (importDebugging) printf("listOfNodes contains %d nodes.\n", lengthList(listOfNodes));
	if (lengthList(errorLog)) {
		deleteNodeList(listOfNodes, true);
		return null;
	}
	// Get list of all GNode trees.
	GNodeList* listOfTrees = getNodeTreesFromNodeList(listOfNodes, file->name, errorLog);
	deleteNodeList(listOfNodes, false);
	if (importDebugging) printf("listOfTrees contains %d records.\n", lengthList(listOfTrees));
	if (lengthList(errorLog)) {
		deleteNodeList(listOfTrees, true);
		return null;
	}
	// Check all keys and the references to them.
	checkKeysAndReferences(listOfTrees, file->name, errorLog);
	if (lengthList(errorLog)) {
		deleteNodeList(listOfTrees, true);
		return null;
	}
	// Create the person and family indexes.
	RecordIndex* personIndex = createRecordIndex();
	RecordIndex* familyIndex = createRecordIndex();
	int otherRecords = 0;
	FORLIST(listOfTrees, element)
		GNodeListEl* el = (GNodeListEl*) element;
		GNode* root = el->node;
		if (recordType(root) == GRPerson)
			addToRecordIndex(personIndex, root->key, root, el->line);
		else if (recordType(root) == GRFamily)
			addToRecordIndex(familyIndex, root->key, root, el->line);
		else
			otherRecords++;
	ENDLIST
	if (importDebugging) {
		printf("The person index holds %d records.\n", sizeHashTable(personIndex));
		printf("The family index holds %d records.\n", sizeHashTable(familyIndex));
		printf("There are %d other records.\n", otherRecords);
	}
	// Create a RecordIndex with all keyed records.
	RecordIndex* recordIndex = createRecordIndex();
	int numAddedToIndex = 0;
	FORLIST(listOfTrees, element)
		GNodeListEl* el = (GNodeListEl*) element;
		GNode* root = el->node;
		if (root->key) {
			numAddedToIndex++;
			addToRecordIndex(recordIndex, root->key, root, el->line);
		}
	ENDLIST
	if (importDebugging) printf("Number added to index is %d.\n", numAddedToIndex);
	// Create Database with first three indexes.
	Database* database = createDatabase(filePath);
	database->recordIndex = recordIndex;
	database->personIndex = personIndex;
	database->familyIndex = familyIndex;
	// Validate persons.
	validatePersons(database, errorLog);
	validateFamilies(database, errorLog);




	return database;
}
//
//	Database* database = createDatabase(filePath); // Create database and add records to it.
//	FORLIST(listOfTrees, element)
//		GNodeListElement* e = (GNodeListElement*) element;
//		storeRecord(database, normalizeRecord(e->node), e->lineNo, errorLog);
//	ENDLIST
//	printf("And now it is time to sort those RootList\n");
//	sortList(database->personRoots);
//	printf("Persons have been sorted\n");
//	sortList(database->familyRoots);
//	printf("Families have been sorted\n");
//	return listOfTrees;
//}

// checkKeysAndReferences checks keys and their references. Creates a table of all keys and checks
// for duplicates. Makes sure all keys found as values refer to records.
void checkKeysAndReferences(GNodeList* records, String name, ErrorLog* errorLog) {
	StringSet* keySet = createStringSet();
	FORLIST(records, element)
		GNodeListEl* el = (GNodeListEl*) element;
		GNode* root = el->node;
		String key = root->key;
		if (!key) continue;
		if (isInSet(keySet, key)) {
			addErrorToLog(errorLog, createError(gedcomError, name, el->line, "duplicate key"));
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
					addErrorToLog(errorLog, error);
					printf("Didn't find key: %s\n", node->value);
				}
		ENDTRAVERSE
	ENDLIST
	if (importDebugging) {
		printf("The length of the key set is %d.\n", lengthSet(keySet));
		printf("The length of the error log is %d.\n", lengthList(errorLog));
		printf("The number of references to keys is %d.\n", numReferences);
		printf("The number of records visited is %d.\n", recordsVisited);
		printf("The number of nodes traversed is %d.\n", nodesTraversed);
		printf("The number of nodes counted is %d.\n", nodesCounted);
	}
}

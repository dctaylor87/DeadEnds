// DeadEnds
//
// database.c holds the functions that provide an in-RAM database for Gedcom records. Each record
// is a GNode tree. The backing store is a Gedcom file. When DeadEnds starts the Gedcom file is
// read and used to build an internal database.
//
// Created by Thomas Wetmore on 10 November 2022.
// Last changed 30 May 2024.

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "refnindex.h"
#include "database.h"
#include "gnode.h"
#include "name.h"
#include "recordindex.h"
#include "stringtable.h"
#include "nameindex.h"
#include "path.h"
#include "errors.h"
#include "rootlist.h"
#include "writenode.h"

extern FILE* debugFile;
extern bool importDebugging;
bool indexNameDebugging = true;
static int keyLineNumber(Database*, CString key);

// createDatabase creates a database.
Database *createDatabase(CString filePath) {
	Database *database = (Database*) stdalloc(sizeof(Database));
	memset (database, 0, sizeof(Database));
	database->filePath = strsave(filePath);
	database->lastSegment = strsave(lastPathSegment(filePath));
	database->personIndex = createRecordIndex();
	database->familyIndex = createRecordIndex();
	database->sourceIndex = createRecordIndex();
	database->eventIndex = createRecordIndex();
	database->otherIndex = createRecordIndex();
	database->nameIndex = createNameIndex();
	database->refnIndex = createRefnIndex();
	database->personRoots = createRootList();
	database->familyRoots = createRootList();
	return database;
}

//  deleteDatabase deletes a database.
void deleteDatabase(Database* database) {
	deleteRecordIndex(database->personIndex);
	deleteRecordIndex(database->familyIndex);
	deleteRecordIndex(database->sourceIndex);
	deleteRecordIndex(database->eventIndex);
	deleteRecordIndex(database->otherIndex);
	deleteNameIndex(database->nameIndex);
}

// writeDatabase writes the contents of a Database to a Gedcom file.
void writeDatabase(String fileName, Database* database) {
	FILE* file = fopen(fileName, "w");
	if (file == null) {
		printf("Can't open file to write database to\n");
		return;
	}
	FORLIST(database->personRoots, element)
		writeGNodeRecord(file, (GNode*) element, false);
	ENDLIST
	FORLIST(database->familyRoots, element)
		writeGNodeRecord(file, (GNode*) element, false);
	ENDLIST
	fclose(file);
}

// numberPersons returns the number of persons in a database.
int numberPersons(Database* database) {
	return database ? sizeHashTable(database->personIndex) : 0;
}

// numberFamilies returns the number of families in a database.
int numberFamilies(Database* database) {
	return database ? sizeHashTable(database->familyIndex) : 0;
}

// numberSources returns the number of sources in a database.
int numberSources(Database* database) {
	return database ? sizeHashTable(database->sourceIndex) : 0;
}

// numberEvents returnw the number of (top level) events in the database.
int numberEvents(Database* database) {
	return database ? sizeHashTable(database->eventIndex) : 0;
}

// numberOthers return the number of other records in the database.
int numberOthers(Database* database) {
	return database ? sizeHashTable(database->otherIndex) : 0;
}

// isEmptyDatabase returns true if the database has no persons or families.
bool isEmptyDatabase(Database* database) {
	return numberPersons(database) + numberFamilies(database) == 0;
}

// keyToPerson gets a person record from a database.
GNode* keyToPerson(CString key, Database* database) {
	RecordIndexEl* element = (RecordIndexEl*) searchHashTable(database->personIndex, key);
	return element ? element->root : null;
}

// keyToFamily gets a family record from a database.
GNode* keyToFamily(CString key, Database* database) {
	RecordIndexEl *element = (RecordIndexEl*) searchHashTable(database->familyIndex, key);
	return element == null ? null : element->root;
}

// keyToSource gets a source record from a database.
GNode* keyToSource(CString key, Database* database) {
	RecordIndexEl* element = (RecordIndexEl*) searchHashTable(database->sourceIndex, key);
	return element ? element->root : null;
}

// keyToEvent gets an event record from a database.
GNode* keyToEvent(CString key, Database* database) {
	RecordIndexEl *element = (RecordIndexEl*) searchHashTable(database->eventIndex, key);
	return element ? element->root : null;
}

// keyToOther gets an other record from a database.
GNode* keyToOther(CString key, Database* database) {
	RecordIndexEl *element = (RecordIndexEl*) searchHashTable(database->otherIndex, key);
	return element ? element->root : null;
}

//  keyToPersonRecord -- Get a person record from a database.
//--------------------------------------------------------------------------------------------------
RecordIndexEl* keyToPersonRecord(CString key, Database *database)
//  key -- Key of person record. The @-signs are not part of the database key.
//  index -- Record index to search for the person.
{
    RecordIndexEl* element = searchHashTable(database->personIndex, key);
    return element;
}

//  keyToFamilyRecord -- Get a family record from a record index.
//--------------------------------------------------------------------------------------------------
RecordIndexEl* keyToFamilyRecord(CString key, Database *database)
//  key -- Key of family record. The @-signs are not part of the key.
//  index -- Record index to search for the family.
{
    //if (debugging) printf("keyToFamilyRecord called with key: %s\n", key);
    RecordIndexEl *element = (RecordIndexEl*) searchHashTable(database->familyIndex, key);
    return element;
}

//  keyToSourceRecord -- Get a source record from the database.
//--------------------------------------------------------------------------------------------------
RecordIndexEl *keyToSourceRecord(CString key, Database *database)
{
    RecordIndexEl* element = searchHashTable(database->sourceIndex, key);
    return element;
}

//  keyToEventRecord -- Get an event record from a database.
//--------------------------------------------------------------------------------------------------
RecordIndexEl *keyToEventRecord(CString key, Database *database)
{
    RecordIndexEl *element = searchHashTable(database->eventIndex, key);
    return element;
}

//  keyToOtherRecord -- Get an other record from a database.
//--------------------------------------------------------------------------------------------------
RecordIndexEl *keyToOtherRecord(CString key, Database *database)
{
    RecordIndexEl *element = searchHashTable(database->otherIndex, key);
    return element;
}

static int count = 0;  // Debugging.

// storeRecord stores a GNode tree in a database by adding it to a RecordIndex.
// lineNumber is the location of the root node in the Gedcom file.
bool storeRecord(Database* database, GNode* root, int lineNumber, ErrorLog* errorLog) {
	//if (importDebugging) fprintf(debugFile, "storeRecord called\n");
	RecordType type = recordType(root);
	//if (importDebugging) fprintf(debugFile, "type of record is %d\n", type);
	if (type == GRHeader || type == GRTrailer) return true;
	if (!root->key) {
		Error *error = createError(syntaxError, database->lastSegment, lineNumber, "This record has no key.");
		addErrorToLog(errorLog, error);
		return false;
	}
	count++;
	String key = root->key;  // MNOTE: insertInRecord copies the key.

	// Duplicate key check done here.
	int previousLine = keyLineNumber(database, key);
	if (previousLine) {
		char scratch[MAXLINELEN];
		sprintf(scratch, "A record with key %s exists at line %d.", key, previousLine);
		Error *error = createError(gedcomError, database->lastSegment, lineNumber, scratch);
		addErrorToLog(errorLog, error);
	}
	switch (type) {
		case GRPerson:
			addToRecordIndex(database->personIndex, key, root, lineNumber);
			insertInRootList(database->personRoots, root);
			return true;
		case GRFamily:
			addToRecordIndex(database->familyIndex, key, root, lineNumber);
			insertInRootList(database->familyRoots, root);
			return true;
		case GRSource:
			addToRecordIndex(database->sourceIndex, key, root, lineNumber);
			return true;
		case GREvent:
			addToRecordIndex(database->eventIndex, key, root, lineNumber);
			return true;
		case GROther:
			addToRecordIndex(database->otherIndex, key, root, lineNumber);
			return true;
		default:
			ASSERT(false);
			return false;
	}
}

// showTableSizes is a debug function that shows the sizes of the database tables.
void showTableSizes(Database *database) {
	printf("Size of personIndex: %d\n", sizeHashTable(database->personIndex));
	printf("Size of familyIndex: %d\n", sizeHashTable(database->familyIndex));
	printf("Size of sourceIndex: %d\n", sizeHashTable(database->sourceIndex));
	printf("Size of eventIndex:  %d\n", sizeHashTable(database->eventIndex));
	printf("Size of otherIndex:  %d\n", sizeHashTable(database->otherIndex));
}

// indexNames indexes all person names in a database.
void oldIndexNames(Database* database) {
	if (indexNameDebugging) fprintf(debugFile, "Start indexNames\n");
	static int count = 0;
	int i, j;
	RecordIndexEl* entry = firstInHashTable(database->personIndex, &i, &j);
	while (entry) { // Loop persons.
		GNode* root = entry->root;
		String recordKey = root->key;
		if (indexNameDebugging) fprintf(debugFile, "indexNames: recordKey: %s\n", recordKey);
		for (GNode* name = NAME(root); name && eqstr(name->tag, "NAME"); name = name->sibling) {
			if (name->value) {
				if (indexNameDebugging) fprintf(debugFile, "indexNames: name->value: %s\n", name->value);
				// MNOTE: nameKey is in data space. It is heapified in insertInNameIndex.
				String nameKey = nameToNameKey(name->value);
				if (indexNameDebugging) fprintf(debugFile, "indexNames: nameKey: %s\n", nameKey);
				insertInNameIndex(database->nameIndex, nameKey, recordKey);
				count++;
			}
		}
		entry = nextInHashTable(database->personIndex, &i, &j);
	}
	if (indexNameDebugging) showNameIndex(database->nameIndex);
	if (indexNameDebugging) printf("The number of names indexed was %d\n", count);
}

// indexNames indexes all person names in a database. NEW VERSION HASN'T BEEN TESTED.
void indexNames(Database* database) {
	if (indexNameDebugging) fprintf(debugFile, "Start indexNames\n");
	static int count = 0;
	FORHASHTABLE(database->personIndex, element)
		RecordIndexEl* entry = element;
		GNode* root = entry->root;
		String recordKey = root->key;
		if (indexNameDebugging) fprintf(debugFile, "indexNames: recordKey: %s\n", recordKey);
		for (GNode* name = NAME(root); name && eqstr(name->tag, "NAME"); name = name->sibling) {
			if (name->value) {
				if (indexNameDebugging) fprintf(debugFile, "indexNames: name->value: %s\n", name->value);
				String nameKey = nameToNameKey(name->value);
				if (indexNameDebugging) fprintf(debugFile, "indexNames: nameKey: %s\n", nameKey);
				insertInNameIndex(database->nameIndex, nameKey, recordKey);
				count++;
			}
		}
	ENDHASHTABLE
	if (indexNameDebugging) showNameIndex(database->nameIndex);
	if (indexNameDebugging) printf("The number of names indexed was %d\n", count);
}

// keyLineNumber checks if a record with a key is in the database; if so it returns the line
// number where the record began in its Gedcom file.
static int keyLineNumber (Database *database, CString key) {
	RecordIndexEl* element = (RecordIndexEl*)searchHashTable(database->personIndex, key);
	if (!element) element = searchHashTable(database->familyIndex, key);
	if (!element) element = searchHashTable(database->sourceIndex, key);
	if (!element) element = searchHashTable(database->eventIndex, key);
	if (!element) element = searchHashTable(database->otherIndex, key);
	if (!element) return 0; // Record doesn't exist.
	return element->lineNumber;
}

// getRecord gets a record from the database given a key.
GNode* getRecord(CString key, Database* database) {
	GNode *root;
	if ((root = keyToPerson(key, database))) return root;
	if ((root = keyToFamily(key, database))) return root;
	if ((root = keyToSource(key, database))) return root;
	if ((root = keyToEvent(key, database))) return root;
	if ((root = keyToOther(key, database))) return root;
	return null;
}

//  Some debugging functions.
void showPersonIndex(Database *database) { showHashTable(database->personIndex, null); }
void showFamilyIndex(Database *database) { showHashTable(database->familyIndex, null); }
int getCount(void) { return count; }


// generateFamilyKey generates a new family key.
String generateFamilyKey(Database* database) {
	return "@Fxxxxxxx@";
}

// generatePersonKey generates a new person key.
String generatePersonKey(Database* database) {
	return "@Ixxxxxxx@";
}

//
//  DeadEnds
//
//  database.c -- Functions that provide an in-RAM database for Gedcom records. Each record is a
//    node tree. The backing store is a Gedcom file. When DeadEnds starts the Gedcom file is read
//    and used to build an internal database of persons, families, and others. Indexing of the
//    records is also done.
//
//  Created by Thomas Wetmore on 10 November 2022.
//  Last changed 13 February 2024.
//

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

static bool debugging = false;
static int keyLineNumber(Database*, String key);
static int keycmp(Word a, Word b) { return strcmp((String) a, (String) b); }
static String keyget(Word a) { return (String) a; }

//  createDatabase -- Create a database.
//--------------------------------------------------------------------------------------------------
Database *createDatabase(CString filePath)
{
	Database *database = (Database*) stdalloc(sizeof(Database));
	database->filePath = strsave(filePath);
	database->lastSegment = strsave(lastPathSegment(filePath));
	database->personIndex = createRecordIndex();
	database->familyIndex = createRecordIndex();
	database->sourceIndex = createRecordIndex();
	database->eventIndex = createRecordIndex();
	database->otherIndex = createRecordIndex();
	database->nameIndex = createNameIndex();
	database->refnIndex = createRefnIndex();
	database->personKeys = createList(keycmp, null, keyget);
	database->familyKeys = createList(keycmp, null, keyget);
	return database;
}

//  deleteDatabase -- Delete a database.
//--------------------------------------------------------------------------------------------------
void deleteDatabase(Database *database)
{
	ASSERT(database);
	deleteRecordIndex(database->personIndex);
	deleteRecordIndex(database->familyIndex);
	deleteRecordIndex(database->sourceIndex);
	deleteRecordIndex(database->eventIndex);
	deleteRecordIndex(database->otherIndex);
	deleteNameIndex(database->nameIndex);
}

//  keyMap -- Table that maps original keys to mapped keys. It is created the first time
//    upateKeyMap is called.
//--------------------------------------------------------------------------------------------------
StringTable *keyMap;

// numberPersons -- Return the number of persons in a database.
//--------------------------------------------------------------------------------------------------
int numberPersons(Database *database)
{
	return database ? sizeHashTable(database->personIndex) : 0;
}

// numberFamilies -- Return the number of families in a database.
//--------------------------------------------------------------------------------------------------
int numberFamilies(Database *database)
{
	return database ? sizeHashTable(database->familyIndex) : 0;
}

// numberSources -- Return the number of sources in a database.
//--------------------------------------------------------------------------------------------------
int numberSources(Database *database)
{
	return database ? sizeHashTable(database->sourceIndex) : 0;
}

//  numberEvents -- Return the number of (top level) events in the database.
//--------------------------------------------------------------------------------------------------
int numberEvents(Database *database)
{
	return database ? sizeHashTable(database->eventIndex) : 0;
}

//  numberOthers -- Return the number of other records in the database.
//--------------------------------------------------------------------------------------------------
int numberOthers(Database *database)
{
	return database ? sizeHashTable(database->otherIndex) : 0;
}


//  isEmptyDatabase -- Return true if the database contains no persons or families.
//-------------------------------------------------------------------------------------------------
bool isEmptyDatabase (Database *database)
{
	return numberPersons(database) + numberFamilies(database) == 0;
}

//  keyToPerson -- Get a person record from a database.
//--------------------------------------------------------------------------------------------------
GNode* keyToPerson(CString key, Database *database)
//  key -- Key of person record.
//  index -- Record index to search for the person.
{
	RecordIndexEl* element = (RecordIndexEl*) searchHashTable(database->personIndex, key);
	return element ? element->root : null;
}

//  keyToFamily -- Get a family record from a record index.
//--------------------------------------------------------------------------------------------------
GNode* keyToFamily(CString key, Database *database)
//  key -- Key of family record.
//  database -- Database to find family in.
{
	//if (debugging) printf("keyToFamily called with key: %s\n", key);
	RecordIndexEl *element = (RecordIndexEl*) searchHashTable(database->familyIndex, key);
	return element == null ? null : element->root;
}

//  keyToSource -- Get a source record from the database.
//--------------------------------------------------------------------------------------------------
GNode *keyToSource(CString key, Database *database)
{
	RecordIndexEl* element = (RecordIndexEl*) searchHashTable(database->sourceIndex, key);
	return element ? element->root : null;
}

//  keyToEvent -- Get an event record from a database.
//--------------------------------------------------------------------------------------------------
GNode *keyToEvent(CString key, Database *database)
{
	RecordIndexEl *element = (RecordIndexEl*) searchHashTable(database->eventIndex, key);
	return element ? element->root : null;
}

//  keyToOther -- Get an other record from a database.
//--------------------------------------------------------------------------------------------------
GNode *keyToOther(CString key, Database *database)
{
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
    if (debugging) printf("keyToFamilyRecord called with key: %s\n", key);
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

//  storeRecord -- Store a Gedcom node tree in the database by adding it to the record index of
//    its type. Return true if the record was added successfully.
//--------------------------------------------------------------------------------------------------
bool storeRecord(Database *database, GNode* root, int lineNumber, ErrorLog *errorLog)
//  database -- Database to add the record to
//  root -- Root of a record tree to store in the database.
//  lineNumber -- Line number in the Gedcom file where the record began.
{
	if (debugging) printf("storeRecord called\n");
	ASSERT(root);
	RecordType type = recordType(root);
	if (debugging) printf("type of record is %d\n", type);
	if (type == GRHeader || type == GRTrailer) return true;  // Ignore HEAD and TRLR records.
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
			insertInRecordIndex(database->personIndex, key, root, lineNumber);
			insertInKeyList(database->personKeys, key);
			return true;
		case GRFamily:
			insertInRecordIndex(database->familyIndex, key, root, lineNumber);
			insertInKeyList(database->familyKeys, key);
			return true;
		case GRSource:
			insertInRecordIndex(database->sourceIndex, key, root, lineNumber);
			return true;
		case GREvent:
			insertInRecordIndex(database->eventIndex, key, root, lineNumber);
			return true;
		case GROther:
			insertInRecordIndex(database->otherIndex, key, root, lineNumber);
			return true;
		default:
			ASSERT(false);
			return false;
	}
}

// tableReport -- Debug function that reports on the sizes of the database tables.
//--------------------------------------------------------------------------------------------------
void showTableSizes(Database *database)
{
	printf("Size of personIndex: %d\n", sizeHashTable(database->personIndex));
	printf("Size of familyIndex: %d\n", sizeHashTable(database->familyIndex));
	printf("Size of sourceIndex: %d\n", sizeHashTable(database->sourceIndex));
	printf("Size of eventIndex:  %d\n", sizeHashTable(database->eventIndex));
	printf("Size of otherIndex:  %d\n", sizeHashTable(database->otherIndex));
}

//  indexNames -- Index all person names in the database. This uses the hash table traverse
//    functions on the person index. For each person all NAME node values are indexed.
//--------------------------------------------------------------------------------------------------
void indexNames(Database* database)
{
	int i, j;  //  State variables.
	static int count = 0;

	if (debugging) printf("Start indexNames\n");

	//  Get the first entry in the person index.
	RecordIndexEl* entry = firstInHashTable(database->personIndex, &i, &j);
	while (entry) {
		GNode* root = entry->root;
		//  MNOTE: The key is heapified in insertInNameIndex.
		String recordKey = root->key;
		//  DEBUG
		if (debugging) printf("indexNames: recordKey: %s\n", recordKey);
		for (GNode* name = NAME(root); name && eqstr(name->tag, "NAME"); name = name->sibling) {
			if (name->value) {
				if (debugging) printf("indexNames: name->value: %s\n", name->value);
				//  MNOTE: nameKey is in data space. It is heapified in insertInNameIndex.
				String nameKey = nameToNameKey(name->value);
				if (debugging) printf("indexNames: nameKey: %s\n", nameKey);
				insertInNameIndex(database->nameIndex, nameKey, recordKey);
				count++;
			}
		}

		//  Get the next entry in the person index.
		entry = nextInHashTable(database->personIndex, &i, &j);
	}
	showNameIndex(database->nameIndex);
	/*if (debugging) */ printf("The number of names indexed was %d\n", count);
}

//  keyLineNumber -- See if a record with the key is in the database. If so return the line number
//    in the original Gedcom file where the record began. Check all five indexes.
//-------------------------------------------------------------------------------------------------
static int keyLineNumber (Database *database, String key)
{
	RecordIndexEl* element = (RecordIndexEl*)searchHashTable(database->personIndex, key);
	if (!element) element = searchHashTable(database->familyIndex, key);
	if (!element) element = searchHashTable(database->sourceIndex, key);
	if (!element) element = searchHashTable(database->eventIndex, key);
	if (!element) element = searchHashTable(database->otherIndex, key);
	if (!element) return 0; // Hasn't been seen before.
	return element->lineNumber;
}

GNode *getRecord(Database *database, String key)
{
	GNode *root;
	if ((root = keyToPerson(key, database))) return root;
	if ((root = keyToFamily(key, database))) return root;
	if ((root = keyToSource(key, database))) return root;
	if ((root = keyToEvent(key, database))) return root;
	if ((root = keyToOther(key, database))) return root;
	return null;
}

//  Some debugging functions.
//--------------------------------------------------------------------------------------------------
void showPersonIndex(Database *database) { showHashTable(database->personIndex, null); }
void showFamilyIndex(Database *database) { showHashTable(database->familyIndex, null); }
int getCount(void) { return count; }

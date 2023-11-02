//
//  DeadEnds
//
//  database.c -- Functions that provide an in-RAM database for Gedcom records. Each record is a
//    node tree. The backing store is a Gedcom file. When DeadEnds starts the Gedcom file is read
//    and used to build an internal database of persons, families, and others. Indexing of the
//    records is also done.
//
//  Created by Thomas Wetmore on 10 November 2022.
//  Last changed 1 November 2023.
//

#include "database.h"
#include "gnode.h"
#include "name.h"
#include "recordindex.h"
#include "stringtable.h"
#include "nameindex.h"

//static bool debugging = true;

//  createDatabase -- Create a database.
//--------------------------------------------------------------------------------------------------
Database *createDatabase(void)
{
	Database *database = (Database*) stdalloc(sizeof(Database));
	database->personIndex = createRecordIndex();
	database->familyIndex = createRecordIndex();
	database->sourceIndex = createRecordIndex();
	database->eventIndex = createRecordIndex();
	database->otherIndex = createRecordIndex();
	database->nameIndex = createNameIndex();
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
	return sizeHashTable(database->personIndex);
}

// numberFamilies -- Return the number of families in a database.
//--------------------------------------------------------------------------------------------------
int numberFamilies(Database *database)
{
	return sizeHashTable(database->familyIndex);
}

// numberSources -- Return the number of sources in a database.
//--------------------------------------------------------------------------------------------------
int numberSources(Database *database)
{
	return sizeHashTable(database->sourceIndex);
}

//  numberEvents -- Return the number of (top level) events in the database.
//--------------------------------------------------------------------------------------------------
int numberEvents(Database *database)
{
	return sizeHashTable(database->eventIndex);
}

//  numberOthers -- Return the number of other records in the database.
//--------------------------------------------------------------------------------------------------
int numberOthers(Database *database)
{
	return sizeHashTable(database->otherIndex);
}

//  keyToPerson -- Get a person record from a database.
//--------------------------------------------------------------------------------------------------
GNode* keyToPerson(String key, Database *database)
//  key -- Key of person record.
//  index -- Record index to search for the person.
{
	RecordIndexEl* element = (RecordIndexEl*) searchHashTable(database->personIndex, key);
	return element ? element->root : null;
}

//  keyToFamily -- Get a family record from a record index.
//--------------------------------------------------------------------------------------------------
GNode* keyToFamily(String key, Database *database)
//  key -- Key of family record.
//  index -- Record index to search for the family.
{
	//if (debugging) printf("keyToFamily called with key: %s\n", key);
	RecordIndexEl *element = (RecordIndexEl*) searchHashTable(database->familyIndex, key);
	return element == null ? null : element->root;
}

//  keyToSource -- Get a source record from the database.
//--------------------------------------------------------------------------------------------------
GNode *keyToSource(String key, Database *database)
{
	RecordIndexEl* element = (RecordIndexEl*) searchHashTable(database->sourceIndex, key);
	return element ? element->root : null;
}

//  keyToEvent -- Get an event record from a database.
//--------------------------------------------------------------------------------------------------
GNode *keyToEvent(String key, Database *database)
{
	RecordIndexEl *element = (RecordIndexEl*) searchHashTable(database->eventIndex, key);
	return element ? element->root : null;
}

static int count = 0;  // Debugging.

//  storeRecord -- Store a gedcom record in the database by adding it to the record index of
//    its type. Return true if the record was added successfully.
//--------------------------------------------------------------------------------------------------
bool storeRecord(Database *database, GNode* root)
//  database -- Database to add the record to
//  root -- Root of a record tree to store in the database.
{
	//if (debugging) printf("storeRecord called\n");
	ASSERT(root);
	RecordType type = recordType(root);
	//if (debugging) printf("type of record is %d\n", type);
	if (type == GRHeader || type == GRTrailer) return true;  // Ignore HEAD and TRLR records.
	ASSERT(root->key);
	count++;
	String key = root->key;  // MNOTE: insertInRecord copies the key.
	switch (type) {
		case GRPerson:
			insertInRecordIndex(database->personIndex, key, root);
			return true;
		case GRFamily:
			insertInRecordIndex(database->familyIndex, key, root);
			return true;
		case GRSource:
			insertInRecordIndex(database->sourceIndex, key, root);
			return true;
		case GREvent:
			insertInRecordIndex(database->eventIndex, key, root);
			return true;
		case GROther:
			insertInRecordIndex(database->otherIndex, key, root);
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

	//  DEBUG
	printf("indexNames\n");

	//  Get the first entry in the person index.
	RecordIndexEl* entry = firstInHashTable(database->personIndex, &i, &j);
	while (entry) {
		GNode* root = entry->root;
		//  MNOTE: The key is heapified in insertInNameIndex.
		String recordKey = root->key;
		//  DEBUG
		//printf("indexNames: recordKey: %s\n", recordKey);
		for (GNode* name = NAME(root); name && eqstr(name->tag, "NAME"); name = name->sibling) {
			if (name->value) {
				//  DEBUG
				//printf("indexNames: name->value: %s\n", name->value);
				//  MNOTE: nameKey is in data space. It is heapified in insertInNameIndex.
				String nameKey = nameToNameKey(name->value);
				//  DEBUG
				//printf("indexNames: nameKey: %s\n", nameKey);
				insertInNameIndex(database->nameIndex, nameKey, recordKey);
				count++;
			}
		}

		//  Get the next entry in the person index.
		entry = nextInHashTable(database->personIndex, &i, &j);
	}
	showNameIndex(database->nameIndex);
	printf("The number of names indexed was %d\n", count);
}

//  Some debugging functions.
//--------------------------------------------------------------------------------------------------
void showPersonIndex(Database *database) { showHashTable(database->personIndex, null); }
void showFamilyIndex(Database *database) { showHashTable(database->familyIndex, null); }
int getCount(void) { return count; }

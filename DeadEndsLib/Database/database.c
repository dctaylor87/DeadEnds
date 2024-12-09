// DeadEnds
//
// database.c has functions that provide an in-RAM database for Gedcom records. Each record is a
// GNode tree. The backing store is a Gedcom file. When DeadEnds starts the Gedcom file is read
// and used to build an internal database.
//
// Created by Thomas Wetmore on 10 November 2022.
// Last changed 4 December 2024.

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

bool indexNameDebugging = false;

// createDatabase creates a database.
Database *createDatabase(CString filePath) {
	Database *database = (Database*) stdalloc(sizeof(Database));
	if (! database)
	  return NULL;
	memset (database, 0, sizeof(Database));
	database->filePath = strsave(filePath);
	database->lastSegment = strsave(lastPathSegment(filePath));
	database->recordIndex = null;
	database->sourceIndex = null;
	database->eventIndex = null;
	database->otherIndex = null;
	database->nameIndex = null;
	database->refnIndex = null;
	database->personRoots = createRootList(); // null?
	database->familyRoots = createRootList(); // null?
	return database;
}

// deleteDatabase deletes a Database.
void deleteDatabase(Database* database) {
	if (database->recordIndex) deleteRecordIndex(database->recordIndex);
	if (database->sourceIndex) deleteRecordIndex(database->sourceIndex);
	if (database->eventIndex) deleteRecordIndex(database->eventIndex);
	if (database->otherIndex) deleteRecordIndex(database->otherIndex);
	if (database->nameIndex) deleteNameIndex(database->nameIndex);
	if (database->refnIndex) deleteRefnIndex(database->refnIndex);
	if (database->personRoots) deleteList(database->personRoots);
	if (database->familyRoots) deleteList(database->familyRoots);
}

// writeDatabase writes the contents of a Database to a Gedcom file.
void writeDatabase(String fileName, Database* database) {
	FILE* file = fopen(fileName, "w");
	if (file == null) {
		printf("Can't open file to write the database\n");
		return;
	}
	FORHASHTABLE(database->recordIndex, element)
		writeGNodeRecord(file, (GNode*) element, false);
	ENDHASHTABLE
	fclose(file);
}

// numberRecordsOfType returns the number of records of given type.
static int numberRecordsOfType(Database* database, RecordType recType) {
	int numRecords = 0;
	FORHASHTABLE(database->recordIndex, element)
		if (recordType((GNode*) element) == recType) numRecords++;
	ENDHASHTABLE
	return numRecords;
}

// numberPersons returns the number of persons in a database.
int numberPersons(Database* database) {
	return numberRecordsOfType(database, GRPerson);
}

// numberFamilies returns the number of families in a database.
int numberFamilies(Database* database) {
	return numberRecordsOfType(database, GRFamily);
}

// numberSources returns the number of sources in a database.
int numberSources(Database* database) {
	return numberRecordsOfType(database, GRSource);
}

// numberEvents returns the number of (top level) events in the database.
int numberEvents(Database* database) {
	return numberRecordsOfType(database, GREvent);
}

// numberOthers return the number of other records in the database.
int numberOthers(Database* database) {
	return numberRecordsOfType(database, GROther);
}

// isEmptyDatabase returns true if the database has no persons or families.
bool isEmptyDatabase(Database* database) {
	return numberPersons(database) + numberFamilies(database) == 0;
}

// keyToRecordOfType returns the root of the GNode tree with given key and record type.
static GNode* keyToRecordOfType(CString key, RecordIndex* index, RecordType recType) {
	GNode* gnode = searchRecordIndex(index, key);
	if (!gnode) return null;
	if (recordType(gnode) != recType) return null;
	return gnode;
}

//// keyToRecord returns the RecordIndexEl with the given key
//RecordIndexEl *keyToRecord (CString key, Database *database) {
//	return ((RecordIndexEl *) searchHashTable (database->recordIndex, key));
//}
  
// keyToPerson gets a person record from a database.
GNode* keyToPerson(CString key, RecordIndex* index) {
	return keyToRecordOfType(key, index, GRPerson);
}

// keyToFamily gets a family record from a database.
GNode* keyToFamily(CString key, RecordIndex* index) {
	return keyToRecordOfType(key, index, GRFamily);
}

// keyToSource gets a source record from a database.
GNode* keyToSource(CString key, RecordIndex* index) {
	return keyToRecordOfType(key, index, GRSource);
}

// keyToEvent gets an event record from a database.
GNode* keyToEvent(CString key, RecordIndex* index) {
	return keyToRecordOfType(key, index, GREvent);
}

// keyToOther gets an other record from a database.
GNode* keyToOther(CString key, RecordIndex* index) {
	return keyToRecordOfType(key, index, GROther);
}

////  keyToPersonRecord -- Get a person record from a database.
////--------------------------------------------------------------------------------------------------
//RecordIndexEl* keyToPersonRecord(CString key, Database *database)
////  key -- Key of person record. The @-signs are not part of the database key.
////  index -- Record index to search for the person.
//{
//    RecordIndexEl* element = searchHashTable(database->personIndex, key);
//    return element;
//}
//
////  keyToFamilyRecord -- Get a family record from a record index.
////--------------------------------------------------------------------------------------------------
//RecordIndexEl* keyToFamilyRecord(CString key, Database *database)
////  key -- Key of family record. The @-signs are not part of the key.
////  index -- Record index to search for the family.
//{
//    //if (debugging) printf("keyToFamilyRecord called with key: %s\n", key);
//    RecordIndexEl *element = (RecordIndexEl*) searchHashTable(database->familyIndex, key);
//    return element;
//}
//
////  keyToSourceRecord -- Get a source record from the database.
////--------------------------------------------------------------------------------------------------
//RecordIndexEl *keyToSourceRecord(CString key, Database *database)
//{
//    RecordIndexEl* element = searchHashTable(database->sourceIndex, key);
//    return element;
//}
//
////  keyToEventRecord -- Get an event record from a database.
////--------------------------------------------------------------------------------------------------
//RecordIndexEl *keyToEventRecord(CString key, Database *database)
//{
//    RecordIndexEl *element = searchHashTable(database->eventIndex, key);
//    return element;
//}
//
////  keyToOtherRecord -- Get an other record from a database.
////--------------------------------------------------------------------------------------------------
//RecordIndexEl *keyToOtherRecord(CString key, Database *database)
//{
//    RecordIndexEl *element = searchHashTable(database->otherIndex, key);
//    return element;
//}

// getNameIndexFromPersons indexes all person names in a database.
NameIndex* getNameIndexFromPersons(RootList* persons) {
	int numNamesFound = 0; // Debugging.
	NameIndex* nameIndex = createNameIndex();
	FORLIST(persons, element) // Loop over persons.
		GNode* root = (GNode*) element;
		String recordKey = root->key; // Key of record, used as is in name index.
		for (GNode* name = NAME(root); name && eqstr(name->tag, "NAME"); name = name->sibling) {
			if (name->value) {
				numNamesFound++; // For debugging.
				String nameKey = nameToNameKey(name->value); // MNOTE: points to static memory.
				insertInNameIndex(nameIndex, nameKey, recordKey);
			}
		}
	ENDLIST
	if (indexNameDebugging) printf("the number of names encountered is %d.\n", numNamesFound);
	return nameIndex;
}

// getNameIndexFromRecordIndex indexes all person names in a RecordIndex and returns the NameIndex.
NameIndex* getNameIndexFromRecordIndex(RecordIndex* index) {
	int numNamesFound = 0; // Debugging.
	NameIndex* nameIndex = createNameIndex();
	FORHASHTABLE(index, element) // Loop over all persons.
		GNode* root = (GNode*) element;
		String recordKey = root->key; // Key of record, used as is in name index.
		for (GNode* name = NAME(root); name && eqstr(name->tag, "NAME"); name = name->sibling) {
			if (name->value) {
				numNamesFound++; // Debugging.
				String nameKey = nameToNameKey(name->value); // MNOTE: points to static memory.
				insertInNameIndex(nameIndex, nameKey, recordKey);
			}
		}
	ENDHASHTABLE
	if (indexNameDebugging) printf("the number of names encountered is %d.\n", numNamesFound);
	return nameIndex;
}


// getRecord gets a record from the database given a key.
GNode* getRecord(CString key, Database* database) {
	return searchRecordIndex(database->recordIndex, key);
}

// summarizeDatabase writes a short summary of a Database to standard output.
void summarizeDatabase(Database* database) {
	if (!database) {
		printf("Database does not exist.\n");
		return;
	}
	printf("Summary of database: %s.\n", database->filePath);
	if (database->recordIndex) {
		printf("\tThe record index has %d records.\n", sizeHashTable(database->recordIndex));
		printf("\tThere are %d persons and %d families in the database.\n",
			   numberPersons(database), numberFamilies(database));
	}
	if (database->nameIndex) {
		int numNames, numRecords;
		getNameIndexStats(database->nameIndex, &numNames, &numRecords);
		printf("\tName index: %d name keys and %d record keys.\n", numNames, numRecords);
	}
}

// Add a record to the database.
// Question: Should this have an additional argument: 'bool update'?
// If update is true, this is an update and the record must already exist.
// If update is false, this is a new record and it must not already exist.
bool storeRecord (Database *database, GNode *root)
{
  switch (recordType (root))
    {
    case GRPerson:
      addToRecordIndex(database->personIndex, root->key, root);
      insertInRootList (database->personRoots, root);
      break;
    case GRFamily:
      addToRecordIndex(database->familyIndex, root->key, root);
      insertInRootList (database->familyRoots, root);
      break;
    case GRSource:
      addToRecordIndex(database->sourceIndex, root->key, root);
      break;
    case GREvent:
      addToRecordIndex(database->eventIndex, root->key, root);
      break;
    case GROther:
      addToRecordIndex(database->otherIndex, root->key, root);
      break;
    default:
      fatal ("unkown record type");
    }
  return true;
}

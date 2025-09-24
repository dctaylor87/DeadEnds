//
//  DeadEnds Library
//
//  key.c has functions that take a record key as an argument and
//  return the corresponding GNode.
//
//  Created by moving functions from Database/datebase.c to eliminate
//  a circular dependency between the Database and Gedcom libraries.

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "hashtable.h"
#include "recordindex.h"
#include "refnindex.h"
#include "database.h"
#include "gedcom.h"
#include "gnode.h"
#include "key.h"

// keyToRecordOfType returns the root of the GNode tree with given key and record type.
static GNode* keyToRecordOfType(CString key, RecordIndex* index, RecordType recType) {
	GNode* gnode = searchRecordIndex(index, key);
	if (!gnode) return null;
	if (recordType(gnode) != recType) return null;
	return gnode;
}

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

//  keyToPersonRecord -- Get a person record from a database.
//--------------------------------------------------------------------------------------------------
GNode* keyToPersonRecord(CString key, Database *database)
//  key -- Key of person record. The @-signs are not part of the database key.
//  index -- Record index to search for the person.
{
  return keyToPerson (key, database->recordIndex);
}

//  keyToFamilyRecord -- Get a family record from a record index.
//--------------------------------------------------------------------------------------------------
GNode* keyToFamilyRecord(CString key, Database *database)
//  key -- Key of family record. The @-signs are not part of the key.
//  index -- Record index to search for the family.
{
  //if (debugging) printf("keyToFamilyRecord called with key: %s\n", key);
  return keyToFamily (key, database->recordIndex);
}

//  keyToSourceRecord -- Get a source record from the database.
//--------------------------------------------------------------------------------------------------
GNode *keyToSourceRecord(CString key, Database *database)
{
  return keyToSource (key, database->recordIndex);
}

//  keyToEventRecord -- Get an event record from a database.
//--------------------------------------------------------------------------------------------------
GNode *keyToEventRecord(CString key, Database *database)
{
  return keyToEvent (key, database->recordIndex);
}

//  keyToOtherRecord -- Get an other record from a database.
//--------------------------------------------------------------------------------------------------
GNode *keyToOtherRecord(CString key, Database *database)
{
  return keyToOther (key, database->recordIndex);
}


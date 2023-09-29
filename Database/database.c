//
//  DeadEnds
//
//  database.c -- Functions that provide an in-RAM database for Gedcom records. Each record is a
//    node tree. The backing store is a Gedcom file. When DeadEnds starts the Gedcom file is read
//    and used to build an internal database of persons, families, and others. Indexing of the
//    records is also done.
//
//  Created by Thomas Wetmore on 10 November 2022.
//  Last changed 11 August 2023.
//

#include "database.h"
#include "gnode.h"
#include "name.h"
#include "recordindex.h"
#include "stringtable.h"
#include "nameindex.h"

static bool debugging = false;

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

// numberPersons -- Return the number of persons in a record index.
//--------------------------------------------------------------------------------------------------
bool personPredicate(Word element)
{
    return recordType(((RecordIndexEl*) element)->root) == GRPerson;
}
int numberPersons(RecordIndex *index)
{
    return iterateHashTableWithPredicate(index, personPredicate);
}

// numberFamilies -- Return the number of families in an index.
//--------------------------------------------------------------------------------------------------
bool familyPredicate(Word element) {
    return recordType(((RecordIndexEl*) element)->root) == GRFamily;
}
int numberFamilies(RecordIndex* index)
{
    return iterateHashTableWithPredicate(index, familyPredicate);
}

// numberSources -- Return the number of sources in an index.
//--------------------------------------------------------------------------------------------------
bool sourcePredicate(Word element)
{
    return recordType(((RecordIndexEl*) element)->root) == GRSource;
}
int numberSources(RecordIndex *index)
{
    return iterateHashTableWithPredicate(index, sourcePredicate);
}

//  numberEvents -- Return the number of (top level) events in the database.
//--------------------------------------------------------------------------------------------------
bool eventPredicate(Word element) {
    return recordType(((RecordIndexEl*) element)->root) == GREvent;
}
int numberEvents(RecordIndex *index)
{
    return iterateHashTableWithPredicate(index, eventPredicate);
}

//  numberOthers -- Return the number of other records in the database.
//--------------------------------------------------------------------------------------------------
bool otherPredicate(Word element) {
    return recordType(((RecordIndexEl*) element)->root) == GROther;
}
int numberOthers(RecordIndex *index)
{
    return iterateHashTableWithPredicate(index, otherPredicate);
}

//  keyToPerson -- Get a person record from a record index.
//--------------------------------------------------------------------------------------------------
GNode* keyToPerson(String key, RecordIndex *index)
//  key -- Key of person record. The @-signs are not part of the database key.
//  index -- Record index to search for the person.
{
    if (debugging) printf("keyToPerson called with key: %s\n", key);
    RecordIndexEl* element = searchHashTable(index, key);
    return element ? element->root : null;
}

//  keyToFamily -- Get a family record from a record index.
//--------------------------------------------------------------------------------------------------
GNode* keyToFamily(String key, RecordIndex *index)
//  key -- Key of family record. The @-signs are not part of the key.
//  index -- Record index to search for the family.
{
    if (debugging) printf("keyToFamily called with key: %s\n", key);
    RecordIndexEl *element = (RecordIndexEl*) searchHashTable(index, key);
    return element == null ? null : element->root;
}

//  keyToSource -- Get a source record from the database.
//--------------------------------------------------------------------------------------------------
GNode *keyToSource(String key, RecordIndex *index)
{
    RecordIndexEl* element = searchHashTable(index, key);
    return element ? element->root : null;
}

//  keyToEvent -- Get an event record from a database.
//--------------------------------------------------------------------------------------------------
GNode *keyToEvent(String key, RecordIndex *index)
{
    RecordIndexEl *element = searchHashTable(index, key);
    return element ? element->root : null;
}

static int count = 0;  // Debugging.

//  storeRecord -- Store a gedcom record in the database by adding it to the record index of
//    its type. Return true if the record was added successfully.
//--------------------------------------------------------------------------------------------------
bool storeRecord(Database *database, GNode* root)
//  root -- Root of a record tree to store in the database.
{
    ASSERT(root && root->key);
    count++;
    RecordType type = recordType(root);
    String key = rmvat(root->key);
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

    //  Get the first entry in the person index.
    RecordIndexEl* entry = firstInHashTable(database->personIndex, &i, &j);
    while (entry) {
        GNode* root = entry->root;
        //  MNOTE: recordKey is static in rmvat. It is heapified in insertInNameIndex.
        String recordKey = rmvat(root->key);
        for (GNode* name = NAME(root); name && eqstr(name->tag, "NAME"); name = name->sibling) {
            if (name->value) {
                //  MNOTE: nameKey is in data space. It is heapified in insertInNameIndex.
                String nameKey = nameToNameKey(name->value);
                insertInNameIndex(database->nameIndex, nameKey, recordKey);
                count++;
            }
        }

        //  Get the next entry in the person index.
        entry = nextInHashTable(database->personIndex, &i, &j);
    }
    showNameIndex(database->nameIndex);
    printf("The number of names printed was %d\n", count);
}

//  Some debugging functions.
//--------------------------------------------------------------------------------------------------
void showPersonIndex(Database *database) { showHashTable(database->personIndex, null); }
void showFamilyIndex(Database *database) { showHashTable(database->familyIndex, null); }
int getCount(void) { return count; }

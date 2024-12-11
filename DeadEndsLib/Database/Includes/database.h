// DeadEnds
//
// database.h is the header file for the Database type.
//
// Created by Thomas Wetmore on 10 November 2022.
// Last changed on 4 December 2024.

#ifndef database_h
#define database_h

#include "standard.h"
#include "hashtable.h"
#include "recordindex.h"
#include "nameindex.h"
#include "refnindex.h"
#include "gnode.h"
#include "errors.h"
#include "rootlist.h"

typedef HashTable RecordIndex;
typedef HashTable NameIndex;
typedef List RootList;

// Database is the structure that hold DeadEnds databases.
typedef struct Database {
	String filePath;  // Path to the Gedcom file this database was built from.
	String lastSegment;  // Last segment of the path for error messages.
	GNode* header; // Root of header record.
	RecordIndex* recordIndex; // Index of all keyed records.
	NameIndex *nameIndex;  // Index of the names of the persons in this database.
	RefnIndex *refnIndex; // Index of the REFN values in this database.
	RootList *personRoots;  // List of all person roots in the database.
	RootList *familyRoots;  // List of all family roots in the database.
#if 0
	CString backupPath;	// path of the most recent backup, if any
#endif
	CString name; // name of the database.
	uint64_t max_indi;
	uint64_t max_fam;
	uint64_t max_even;
	uint64_t max_sour;
	uint64_t max_othr;
	uint8_t flags;
	uint8_t longestKeySeen; // Length of longest key added to database.
} Database;

extern Database *currentDatabase;

/* database flags */
#define DATABASE_DIRTY		0x1 /* modified since last written out */
#define DATABASE_READONLY	0x2 /* is this useful? */

#define database_dirty(database)	(database->flags & DATABASE_DIRTY)
#define database_readonly(database)	(database->flags & DATABASE_READONLY)

Database *createDatabase(CString fileName); // Create an empty database.
void deleteDatabase(Database*); // Delete a database.
void writeDatabase(String fileName, Database*);

void indexNames(Database*);      // Index person names after reading the Gedcom file.
int numberPersons(Database*);    // Return the number of persons in the database.
int numberFamilies(Database*);   // Return the number of families in the database.
int numberSources(Database*);    // Return the number of sources in the database.
int numberEvents(Database*);     // Return the number of events in the database.
int numberOthers(Database*);     // Return the number of other records in the database.
bool isEmptyDatabase(Database*);  // Return true if the database has not persons or families.
GNode *keyToRecord (CString key, Database *); // Return a key's record
GNode *keyToPerson(CString key, RecordIndex*); // Get a person from record index.
GNode *keyToFamily(CString key, RecordIndex*); // Get a family GNode from a RecordIndex.
GNode *keyToSource(CString key, RecordIndex*); // Get a source record from the database.
GNode *keyToEvent(CString key, RecordIndex*); // Get an event record from the database.
GNode *keyToOther(CString key, RecordIndex*); // Get an other record from the database.
GNode *getRecord(CString key, Database*);  // Get an arbitraray record from the database.

// oldIndexNames indexes all person names in a database.
extern void oldIndexNames(Database* database);

//  Get a person record from the database.
extern GNode *keyToPersonRecord(CString key, Database*);
//  Get a family record from the database.
extern GNode *keyToFamilyRecord(CString key, Database*);
//  Get a source record from the database.
extern GNode *keyToSourceRecord(CString key, Database*);
//  Get an event record from the database.
extern GNode *keyToEventRecord(CString key, Database*);
//  Get an other record from the database.
extern GNode *keyToOtherRecord(CString Key, Database*);

bool storeRecord(Database*, GNode*); // Add a record to the database.
NameIndex* getNameIndexFromPersons(RootList*);
void summarizeDatabase(Database*);

extern int getCount(void);

String generateFamilyKey(Database*);
String generatePersonKey(Database*);

#endif // database_h

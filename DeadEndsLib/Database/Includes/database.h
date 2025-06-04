//
//  DeadEnds
//
//  database.h is the header file for the Database type.
//
//  Created by Thomas Wetmore on 10 November 2022.
//  Last changed on 3 June 2025.
//

#ifndef database_h
#define database_h

#include "standard.h"

typedef struct Database Database; // Only needed because of where DatabaseAction is defined.
typedef struct GNode GNode;
typedef struct HashTable HashTable;
typedef struct List List;

typedef HashTable IntegerTable;
typedef HashTable NameIndex;
typedef HashTable RecordIndex;
typedef HashTable RefnIndex;
typedef List RootList;
typedef List ErrorLog;

// DBaseAction is a "Database action" that customizes Database processing.
// NOTE: THIS IS A GOOD IDEA THAT IS NOT YET INCORPORTATED.
typedef void (*DBaseAction)(Database*, ErrorLog*);

// Database is the structure that hold DeadEnds databases.
typedef struct Database {
	CString path;  // Path to Gedcom file this Database was built from.
	CString name; // Use last segment of the path for the name of the Database.
	GNode* header; // Root of header record.
	bool dirty; // Dirty flag.
	RecordIndex* recordIndex; // Index of all keyed records.
	NameIndex *nameIndex; // Index of the names of the persons in this database.
	RefnIndex *refnIndex; // Index of the REFN values in this database.
	RootList *personRoots; // List of all person roots in the database.
	RootList *familyRoots; // List of all family roots in the database.
	RootList *sourceRoots; // List of all source roots in the database.
	RootList *eventRoots;  // List of all the event roots in the database.
	RootList *otherRoots;  // List of all the other roots in the database.
#if 0
	CString backupPath;	// path of the most recent backup, if any
#endif
	/* the following five fields only make sense when the database has
	   been rekeyed. */
	uint64_t db_max_person;	// current max for person keys
	uint64_t db_max_family;	// current max for family keys
	uint64_t db_max_event;	// current max for event keys
	uint64_t db_max_source;	// current max for source keys
	uint64_t db_max_other;	// current max for other keys

	bool db_dirty;		// if true, database is dirty
} Database;

extern Database *currentDatabase;

// returns true if database is dirty; false if database is clean
extern bool databaseIsDirty (Database *database);

// sets database dirty state to value of 'dirty'
extern void setDatabaseDirty (Database *database, bool dirty);

// gets database dirty state
extern bool getDatabaseDirty (Database *database);

Database *createDatabase(CString fileName, RootList*, IntegerTable*, ErrorLog*); // Create a database.
void deleteDatabase(Database*); // Delete a database.
void writeDatabase(String fileName, Database*);

void indexNames(Database*);      // Index person names after reading the Gedcom file.
int numberPersons(Database*);    // Return the number of persons in the database.
int numberFamilies(Database*);   // Return the number of families in the database.
int numberSources(Database*);    // Return the number of sources in the database.
int numberEvents(Database*);     // Return the number of events in the database.
int numberOthers(Database*);     // Return the number of other records in the database.
bool isEmptyDatabase(Database*);  // Return true if the database has not persons or families.
//GNode *keyToRecord (CString key, Database *); // Return a key's record
GNode *keyToPerson(CString key, RecordIndex*); // Get a person from record index.
GNode *keyToFamily(CString key, RecordIndex*); // Get a family GNode from a RecordIndex.
GNode *keyToSource(CString key, RecordIndex*); // Get a source record from the database.
GNode *keyToEvent(CString key, RecordIndex*); // Get an event record from the database.
GNode *keyToOther(CString key, RecordIndex*); // Get an other record from the database.
GNode *getRecord(CString key, RecordIndex*);  // Get an arbitraray record from the database.

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
void summarizeDatabase(Database*);

extern int getCount(void);

String generateFamilyKey(Database*);
String generatePersonKey(Database*);

#endif // database_h

//
//  DeadEnds
//
//  database.h
//
//  Created by Thomas Wetmore on 10 November 2022.
//  Last changed on 22 February 2024.
//

#ifndef database_h
#define database_h

#include "standard.h"
#include "hashtable.h"
#include "recordindex.h"
#include "nameindex.h"
#include "refnindex.h"
#include "gnode.h"
#include "errors.h"
#include "keylist.h"

typedef HashTable RecordIndex;
typedef List KeyList;

//  Database -- Database structure for genealogical data encoded in Gedcom form.
//--------------------------------------------------------------------------------------------------
typedef struct Database {
    String filePath;  // Path to the Gedcom file this database was built from.
    String lastSegment;  // Last segment of the path for error messages.
    RecordIndex *personIndex;  // Index of persons in this database.
    RecordIndex *familyIndex;  // Index of families in this database.
    RecordIndex *sourceIndex;  // Index of sources in this database.
    RecordIndex *eventIndex;  // Index of events in this database.
    RecordIndex *otherIndex;  // Index of other records in this database.
    NameIndex *nameIndex;  // Index of the names of the persons in this database.
    RefnIndex *refnIndex; // Index of REFN values in this database.
    KeyList *personKeys;  // List of all person keys in the database.
    KeyList *familyKeys;  // List of all family keys in the database.
#if 0
    CString backupPath;	// path of the most recent backup, if any
#endif
} Database;

extern Database *currentDatabase;

Database *createDatabase(CString fileName); // Create an empty database.
void deleteDatabase(Database*); // Delete a database.

void indexNames(Database*);      // Index person names after reading the Gedcom file.
int numberPersons(Database*);    // Return the number of persons in the database.
int numberFamilies(Database*);   // Return the number of families in the database.
int numberSources(Database*);    // Return the number of sources in the database.
int numberEvents(Database*);     // Return the number of events in the database.
int numberOthers(Database*);     // Return the number of other records in the database.
bool isEmptyDatabase(Database*);  // Return true if the database has not persons or families.
GNode *keyToPerson(CString key, Database*); // Get a person record the database.
GNode *keyToFamily(CString key, Database*); // Get a family record from the database.
GNode *keyToSource(CString key, Database*); // Get a source record from the database.
GNode *keyToEvent(CString key, Database*); // Get an event record from the database.
GNode *keyToOther(CString Key, Database*); // Get an other record from the database.
GNode *getRecord(Database*, String key);  // Get an arbitraray record from the database.

//  Get a person record from the database.
extern RecordIndexEl *keyToPersonRecord(CString key, Database*);
//  Get a family record from the database.
extern RecordIndexEl *keyToFamilyRecord(CString key, Database*);
//  Get a source record from the database.
extern RecordIndexEl *keyToSourceRecord(CString key, Database*);
//  Get an event record from the database.
extern RecordIndexEl *keyToEventRecord(CString key, Database*);
//  Get an other record from the database.
extern RecordIndexEl *keyToOtherRecord(CString Key, Database*);

bool storeRecord(Database*, GNode*, int lineno, ErrorLog*); // Add a record to the database.
void showTableSizes(Database*);  // Show the sizes of the database tables. Debugging.
void showPersonIndex(Database*); // Show the person index. Debugging.
void showFamilyIndex(Database*); // Show the family index. Debugging.
//void insertInKeyList(List*, String);

extern int getCount(void);

#endif // database_h

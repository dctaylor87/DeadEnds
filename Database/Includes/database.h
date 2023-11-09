//
//  DeadEnds
//
//  database.h
//
//  Created by Thomas Wetmore on 10 November 2022.
//  Last changed on 7 October 2023.
//

#ifndef database_h
#define database_h

#include "standard.h"
#include "hashtable.h"
#include "recordindex.h"
#include "nameindex.h"
#include "gnode.h"

typedef HashTable RecordIndex;

//  Database -- Database structure for genealogical data encoded in Gedcom form.
//--------------------------------------------------------------------------------------------------
typedef struct Database {
    String fileName;  // Name of Gedcom file this database was built from.
    RecordIndex *personIndex;
    RecordIndex *familyIndex;
    RecordIndex *sourceIndex;
    RecordIndex *eventIndex;
    RecordIndex *otherIndex;
    NameIndex *nameIndex;
} Database;

extern Database *theDatabase;

Database *createDatabase(String fileName);  //  Create an empty database.
void deleteDatabase(Database*);  //  Delete a database.

void indexNames(Database*);      //  Index person names after reading the Gedcom file.
int numberPersons(Database*);    //  Return the number of persons in the database.
int numberFamilies(Database*);   //  Return the number of families in the database.
int numberSources(Database*);    //  Return the number of sources in the database.
int numberEvents(Database*);     //  Return the number of events in the database.
int numberOthers(Database*);     //  Return the number of other records in the database.
GNode *keyToPerson(CString key, Database*);  //  Get a person node the database.
GNode *keyToFamily(CString key, Database*);  //  Get a family node from the database.
GNode *keyToSource(CString key, Database*);  //  Get a source node from the database.
GNode *keyToEvent(CString key, Database*);   //  Get an event node from the database.
GNode *keyToOther(CString Key, Database*);   //  Get an other node from the database.

//  Get a person record from the database.
RecordIndexEl *keyToPersonRecord(CString key, Database*);
//  Get a family record from the database.
RecordIndexEl *keyToFamilyRecord(CString key, Database*);
//  Get a source record from the database.
RecordIndexEl *keyToSourceRecord(CString key, Database*);
//  Get an event record from the database.
RecordIndexEl *keyToEventRecord(CString key, Database*);
//  Get an other record from the database.
RecordIndexEl *keyToOtherRecord(CString Key, Database*);

bool storeRecord(Database*, GNode*);        //  Add a record to the database.
void showTableSizes(Database*);          //  Show the sizes of the database tables. Debugging.
void showPersonIndex(Database*);      //  Show the person index. Debugging.
void showFamilyIndex(Database*);      //  Show the family index. Debugging.
extern int getCount(void);

#endif // database_h

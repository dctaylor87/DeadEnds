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

Database *createDatabase(String fileName);  //  Create an empty database.
void deleteDatabase(Database*);  //  Delete a database.

void indexNames(Database*);      //  Index person names after reading the Gedcom file.
int numberPersons(Database*);    //  Return the number of persons in the database.
int numberFamilies(Database*);   //  Return the number of families in the database.
int numberSources(Database*);    //  Return the number of sources in the database.
int numberEvents(Database*);     //  Return the number of events in the database.
int numberOthers(Database*);     //  Return the number of other records in the database.
GNode *keyToPerson(String key, Database*);  //  Get a person record the database.
GNode *keyToFamily(String key, Database*);  //  Get a family record from the database.
GNode *keyToSource(String key, Database*);  //  Get a source record from the database.
GNode *keyToEvent(String key, Database*);   //  Get an event record from the database.
GNode *keyToOther(String Key, Database*);   //  Get an other record from the database.
bool storeRecord(Database*, GNode*);        //  Add a record to the database.
void showTableSizes(Database*);          //  Show the sizes of the database tables. Debugging.
void showPersonIndex(Database*);      //  Show the person index. Debugging.
void showFamilyIndex(Database*);      //  Show the family index. Debugging.

#endif // database_h

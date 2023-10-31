//
//  DeadEnds Project
//
//  recordindex.h -- Defines the record index as a hash table.
//
//  Created by Thomas Wetmore on 29 November 2022.
//  Last changed on 31 October 2023.
//

#ifndef recordindex_h
#define recordindex_h

#include "gnode.h"
#include "hashtable.h"

//  RecordIndexEl -- An element of a record index bucket.
//--------------------------------------------------------------------------------------------------
typedef struct RecordIndexEl {
	GNode *root;  //  The root node of the record.
}  RecordIndexEl;

//  RecordIndex -- A record index is a hash table.
//--------------------------------------------------------------------------------------------------
typedef HashTable RecordIndex;

// User interface to RecordIndex.
//--------------------------------------------------------------------------------------------------
RecordIndex *createRecordIndex(void);                   //  Create a record index.
void deleteRecordIndex(RecordIndex*);                   //  Delete a record index.
void insertInRecordIndex(RecordIndex*, String, GNode*); //  Add an entry to a RecordIndex.
GNode* searchRecordIndex(RecordIndex*, String);         //  Search for an entry in a RecordIndex.
void showRecordIndex(RecordIndex*);                     //  Show the contents of record index.

#endif // recordindex_h

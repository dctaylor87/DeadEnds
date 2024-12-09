// DeadEnds Project
//
// recordindex.h defines RecordIndex as a HashTable.
//
// Created by Thomas Wetmore on 29 November 2022.
// Last changed on 4 December 2024.

#ifndef recordindex_h
#define recordindex_h

/* forward reference needed */
struct RecordIndexEl;
typedef struct RecordIndexEl RecordIndexEl;

#include "gnode.h"
#include "hashtable.h"

// A RecordIndex is a HashTable where the elements are GNodes pointers.
typedef HashTable RecordIndex;

// Interface to RecordIndex.
RecordIndex *createRecordIndex(void);
void deleteRecordIndex(RecordIndex*);
void addToRecordIndex(RecordIndex*, GNode* root);
GNode* searchRecordIndex(RecordIndex*, CString);
void showRecordIndex(RecordIndex*);

extern int getRecordInsertCount(void); // Return the record insert count.  For debugging.
extern void newShowRecordIndex(RecordIndex* index);
extern void addrefRecord (RecordIndexEl *element);
extern void releaseRecord (RecordIndexEl *element);

#endif // recordindex_h

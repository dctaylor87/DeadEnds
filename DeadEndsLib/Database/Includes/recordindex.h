// DeadEnds Project
//
// recordindex.h defines RecordIndex as a HashTable.
//
// Created by Thomas Wetmore on 29 November 2022.
// Last changed on 10 December 2024.

#ifndef recordindex_h
#define recordindex_h

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
//extern void addrefRecord (GNode *element);
//extern void releaseRecord (GNode *element);

// FORRECORDINDEX iterates a RecordIndex returning only GNode*s of a specific type.
#define FORRECORDINDEX(table, gnode, type) {\
		int __i = 0, __j = 0;\
		HashTable *__table = table;\
		GNode* gnode = null;\
		GNode* __gnode = (GNode*) firstInHashTable(__table, &__i, &__j);\
		for(; __gnode; __gnode = (GNode*) nextInHashTable(__table, &__i, &__j)) {\
		if (recordType(__gnode) != type) continue;\
			gnode = __gnode;
#define ENDRECORDINDEX }}

#endif // recordindex_h



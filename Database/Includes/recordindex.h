// DeadEnds Project
//
// recordindex.h defines RecordIndex as a HashTable.
//
// Created by Thomas Wetmore on 29 November 2022.
// Last changed on 13 May 2024.

#ifndef recordindex_h
#define recordindex_h

/* forward reference needed */
struct RecordIndexEl;
typedef struct RecordIndexEl RecordIndexEl;

#include "gnode.h"
#include "hashtable.h"

// A RecordIndexEl in an element in a RecordIndex. It holds the root node of a Gedcom record and
// the line number where the record was defined. The element's key is the key of the root node.
typedef struct RecordIndexEl {
	GNode *root;
	int lineNumber;
	int refcount;	// Number of references to the element
}  RecordIndexEl;

// A RecordIndex is a HashTable of RecordIndexEls.
typedef HashTable RecordIndex;

// Interface to RecordIndex.
RecordIndex *createRecordIndex(void);
void deleteRecordIndex(RecordIndex*);
void addToRecordIndex(RecordIndex*, String, GNode*, int lineNo);
GNode* searchRecordIndex(RecordIndex*, String);
void showRecordIndex(RecordIndex*);

extern int getRecordInsertCount(void); // Return the record insert count.  For debugging.
extern void newShowRecordIndex(RecordIndex* index);
extern void addrefRecord (RecordIndexEl *element);
extern void releaseRecord (RecordIndexEl *element);

#endif // recordindex_h

//  DeadEnds Library
//
//  recordindex.c holds the functions that implement RecordIndex, a HashTable that maps Gedcom
//  record keys to the roots of the GNode trees with those keys.
//
//  Created by Thomas Wetmore on 29 November 2022.
//  Last changed on 4 June 2025.

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "standard.h"
#include "gnode.h"
#include "hashtable.h"
#include "refnindex.h"
#include "recordindex.h"
#include "list.h"
#include "sort.h"
//#include "gedcom.h"

#define numRecordIndexBuckets 2047
#define brownnose false

// compare is the compare function for record keys.
static int compare(CString left, CString right) {
	return compareRecordKeys(left, right);
}

// getKey returns the key of a RecordIndex element.
static CString getKey(const void* element) {
	return ((GNode*) element)->key;
}

// createRecordIndex creates a RecordIndex. The delete function is null to prevent the trees
// themselves from being deleted.
RecordIndex *createRecordIndex(void) {
	return createHashTable(getKey, compare, null, numRecordIndexBuckets);
}

// deleteRecordIndex deletes a RecordIndex.
void deleteRecordIndex(RecordIndex *index) {
	deleteHashTable(index);
}

// addToRecordIndex adds a GNode record tree to a RecordIndex.
void addToRecordIndex(RecordIndex* index, GNode* root) {
	ASSERT(root && root->key);
	addToHashTable(index, root, false);
}

// searchRecordIndex searches a RecordIndex by key and returns the associated GNode tree.
GNode* searchRecordIndex(RecordIndex *index, CString key) {
	return (GNode*) searchHashTable(index, key);
}

// showRecordIndex shows the contents of a RecordIndex. For debugging.
void showRecordIndex(RecordIndex* index) {
	FORHASHTABLE(index, element)
		printf("Key %s\n", ((GNode*) element)->key);
	ENDHASHTABLE
}

// addrefRecord -- increment reference count of a GNode.

void
addrefRecord (GNode *element)
{
  if (! element)
    return;

  element->refcount++;
}

// releaseRecord -- decrement reference count of a GNode.
// If it reaches zero, we do NOT free it.
// If it is zero before we decrement, we ABORT.

void
releaseRecord (GNode *element)
{
#if 1				/* XXX */
  return;
#else
  if (! element)
    return;

  ASSERT(element->refcount);
  element->refcount--;
#endif
}

// DeadEnds Project
//
// refnindex.c -- Handle user reference indexing.
//
// Gedcom records can have 1 REFN nodes whose values give records unique identifiers.
//
// Created by Thomas Wetmore on 16 December 2023.
// Last changed on 16 May 2024.

#include <stdint.h>

#include "refnindex.h"
#include "gedcom.h"

static int numRefnIndexBuckets = 1024;

// searchRefnIndex searches a RefnIndex for a 1 REFN value and returns the key of the record with
// that value.
CString searchRefnIndex(RefnIndex* index, CString refn) {
	if (!index || !refn) return null;
	RefnIndexEl* el = (RefnIndexEl*) searchHashTable(index, refn);
	return el ? el->key : null;
}

// createRefnIndexEl creates a new reference index entry.
RefnIndexEl *createRefnIndexEl(CString refn, CString key) {
	RefnIndexEl *el = (RefnIndexEl*) malloc(sizeof(RefnIndexEl));
	el->refn = refn;
	el->key = key;
	return el;
}

// showRefnIndex show a RefnIndex, for debugging.
void showRefnIndex(RefnIndex* index) {
	printf("showRefnIndex: Write me\n");
}

// compare compares two record keys.
static int compare (CString a, CString b) {
	return strcmp(a, b);
}

// getKey returns the key of a RefnIndexEl, a 1 REFN value.
static CString getKey(void* a) {
	return ((RefnIndexEl*) a)->key;
}

// delete frees a RefnIndexEl.
static void delete(void* element) {
	free(element);
}

// createRefnIndex creates a RefnIndex.
RefnIndex *createRefnIndex(void) {
	return (RefnIndex*) createHashTable(getKey, compare, delete, numRefnIndexBuckets);
}

// deleteRefnIndex deletes a RefnIndex.
void deleteRefnIndex(RefnIndex *index) {
	deleteHashTable(index);
}

// addToRefnIndex adds a new RefnIndexEl in a RefnIndex. Returns true on success; returns false
// if the REFN value was already in the table.
bool addToRefnIndex(RefnIndex *index, String refn, String key) {
	RefnIndexEl* element = createRefnIndexEl(strsave(refn), strsave(key));
	bool added = addToHashTableIfNew(index, element);
	if (added) return true;
	delete(element);
	return false;
}

/* getRefn -- searches the index for a mapping for refn, if found, returns it.
   If not found, null is returned.  */

CString
getRefn (CString refn, Database *database)
{
  return searchRefnIndex (database->refnIndex, refn);
}

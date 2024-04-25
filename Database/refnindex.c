//
// DeadEnds Project
//
// refnindex.c -- Handle user reference indexing.
//
// Gedcom records can have 1 REFN nodes whose values give records unique identifiers.
//
// Created by Thomas Wetmore on 16 December 2023.
// Last changed on 3 April 2024.

#include <stdint.h>

#include "refnindex.h"
#include "gedcom.h"

static int numRefnIndexBuckets = 1024;

// searchRefnIndex searches a RefnIndex for a 1 REFN value and returns the key of the record with
// that value.
CString searchRefnIndex(RefnIndex *index, CString refn) {
	RefnIndexEl *el = (RefnIndexEl*) searchHashTable(index, refn);
	return el ? el->key : null;
}

// createRefnIndexEl creates a new reference index entry.
RefnIndexEl *createRefnIndexEl(CString refn, CString key) {
	RefnIndexEl *el = (RefnIndexEl*) malloc(sizeof(RefnIndexEl));
	el->refn = refn;
	el->key = key;
	return el;
}

// showRefnIndex -- Show a RefnIndex, for debugging.
//--------------------------------------------------------------------------------------------------
void showRefnIndex(RefnIndex *index)
{
	printf("showRefnIndex: Write me\n");
}

// compare compares two record keys.
// NOTE: There is a special key compare function that could be used here, though probably
// unnessary
static int compare (String a, String b) {
	return strcmp(a, b);
}

// getKey returns the key of a RefnIndexEl, which is a 1 REFN value.
static String getKey(void* a) {
	return ((RefnIndexEl*) a)->key;
}

// delete frees a RefnIndexEl.
static void delete(void* element) {
	RefnIndexEl* e = (RefnIndexEl*) element;
	free(e->key);
	free(e->refn);
	free(e);
}

// createRefnIndex creates a RefnIndex.
RefnIndex *createRefnIndex(void) {
	return (RefnIndex*) createHashTable(getKey, compare, delete, numRefnIndexBuckets);
}

// deleteRefnIndex deletes a RefnIndex.
void deleteRefnIndex(RefnIndex *index) {
	deleteHashTable(index);
}

// addToRefnIndex tries to add a new RefnIndexEl in a RefnIndex. Returns true on success; returns
// false if the REFN value was already in the table, which should be treated as an error.
bool addToRefnIndex(RefnIndex *index, String refn, String key) {
	RefnIndexEl* element = createRefnIndexEl(strsave(refn), strsave(key));
	bool added = addToHashTableIfNew(index, element);
	if (added) return true;
	delete(element);
	return false;
}

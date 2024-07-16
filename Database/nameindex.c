// DeadEnds Project
//
// nameindex.c implements the NameIndex, an index that maps Gedcom name keys to the sets of person
// record keys that have the names.
//
// Created by Thomas Wetmore on 26 November 2022.
// Last changed on 5 July 2024.

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "refnindex.h"
#include "nameindex.h"
#include "name.h"
#include "sort.h"
#include "set.h"
#include "gedcom.h"

// Local functions
static NameIndexEl* createNameIndexEl(CString nameKey, String recordKey);
static int numNameIndexBuckets = 2048;

// getKey gets the name key of a NameIndex element.
static CString getKey(void* element) {
	return ((NameIndexEl*) element)->nameKey;
}

// compare compares two name keys.
static int compare(CString a, CString b) {
	return strcmp(a, b);
}

// delete frees a NameIndex element.
static void delete(void* element) {
	NameIndexEl *nameEl = (NameIndexEl*) element;
	stdfree(nameEl->nameKey);
	deleteSet(nameEl->recordKeys);
	stdfree(nameEl);
}

// createNameIndex creates a NameIndex.
NameIndex *createNameIndex(void) {
	return createHashTable(getKey, compare, delete, numNameIndexBuckets);
}

// deleteNameIndex deletes a name index.
void deleteNameIndex(NameIndex *nameIndex) {
	deleteHashTable(nameIndex);
}

// insertInNameIndex adds a (name key, person key) relationship to a NameIndex.
void insertInNameIndex(NameIndex* index, CString nameKey, String recordKey) {
	//printf("insertInNameIndex: nameKey, personKey: %s, %s\n", nameKey, personKey); // DEBUG
	NameIndexEl* element = (NameIndexEl*) searchHashTable(index, nameKey);
	if (!element) {
		element = createNameIndexEl(nameKey, recordKey);
		addToHashTable(index, element, true);
	}
	Set* recordKeys = element->recordKeys;
	if (!isInSet(recordKeys, recordKey)) {
		addToSet(recordKeys, recordKey);
	}
}

// removeFromNameIndex removes a key from a name entry.
// returns true if found (and removed); returns false if not present.
bool removeFromNameIndex (NameIndex *index, CString nameKey, CString recordKey)
{
  NameIndexEl *element = (NameIndexEl *) searchHashTable (index, nameKey);

  if (! element)
    return false;		// should not happen

  Set *recordKeys = element->recordKeys;
  if (! isInSet (recordKeys, recordKey))
    return false;		// should not happen

  removeFromSet (recordKeys, recordKey);

  return true;
}

// searchNameIndex searches NameIndex for a name and returns the record keys that have the name.
Set* searchNameIndex(NameIndex* index, CString name) {
	String nameKey = nameToNameKey(name);
	NameIndexEl* element = searchHashTable(index, nameKey);
	return element == null ? null : element->recordKeys;
}

// showNameIndex shows the contents of a name index.
static void showSetElement(void* setEl) {
	printf("  %s\n", (String) setEl);
}
static void showElement(void* element) {
	Set* recordKeys = ((NameIndexEl*)element)->recordKeys;
	iterateSet(recordKeys, showSetElement);
}
void showNameIndex(NameIndex* index) {
	showHashTable(index, showElement);
}

// getSetKey gets the key of a Set element.
static CString getSetKey(void* element) {
	return (CString) element;
}

// compareSetKeys compares two keys of Set elements.
static int compareSetKeys(CString a, CString b) {
	return compareRecordKeys(a, b);
}

// createNameIndexEl creates and returns a NameIndexEl.
static NameIndexEl* createNameIndexEl(CString nameKey, String recordKey) {
	NameIndexEl* el = (NameIndexEl*) stdalloc(sizeof(NameIndexEl));
	if (! el)
	  return NULL;
	memset (el, 0, sizeof(NameIndexEl));
	el->nameKey = strsave(nameKey);  // MNOTE: nameKey is in data space.
	el->recordKeys = createSet(getSetKey, compareSetKeys, null);
	return el;
}

// getNameIndexStats returns statistics about the NameIndex. For testing and debugging.
void getNameIndexStats(NameIndex* index, int* pnumNameKeys, int* pnumRecordKeys) {
	int numNameKeys = 0;
	int numRecordKeys = 0;
	FORHASHTABLE(index, element)
		numNameKeys++;
		NameIndexEl* el = (NameIndexEl*) element;
		numRecordKeys += lengthSet(el->recordKeys);
	ENDHASHTABLE
	*pnumNameKeys = numNameKeys;
	*pnumRecordKeys = numRecordKeys;
}

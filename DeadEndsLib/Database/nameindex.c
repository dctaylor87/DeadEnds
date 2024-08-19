// DeadEnds Project
//
// nameindex.c implements the NameIndex, an index that maps Gedcom name keys to the Sets of person
// record keys that have the names.
//
// Created by Thomas Wetmore on 26 November 2022.
// Last changed on 18 August 2024.

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "gnode.h"
#include "refnindex.h"
#include "nameindex.h"
#include "name.h"
#include "sort.h"
#include "set.h"
#include "gedcom.h"

// Local functions
static NameIndexEl* createNameIndexEl(CString nameKey);
static bool nameIndexDebugging = false;
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
// MNOTE: the nameKey is freed.
// MNOTE: the recordKeys Set is freed (the recordKeys it contains are not).
// MNOTE: the element itself is freed.
static void delete(void* element) {
	NameIndexEl *el = (NameIndexEl*) element;
	stdfree(el->nameKey);
	deleteSet(el->recordKeys);
	stdfree(el);
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
// MNOTE: nameKey is in static memory; it is saved if createNameIndexEl is called.
// MNOTE: recordKey is the record key from the database; it is not saved.
void insertInNameIndex(NameIndex* index, CString nameKey, String recordKey) {
	if (nameIndexDebugging)
		printf("insertInNameIndex: nameKey, personKey: %s, %s\n", nameKey, recordKey);
	NameIndexEl* element = (NameIndexEl*) searchHashTable(index, nameKey); // Name key seen before?
	if (!element) { // No.
		element = createNameIndexEl(nameKey); // MNOTE: createNameIndexEl saves nameKey.
		addToHashTable(index, element, true);
	}
	Set* recordKeys = element->recordKeys;
	if (!isInSet(recordKeys, recordKey)) {
		addToSet(recordKeys, recordKey); // MNOTE: recordKey from Database stored in index as is.
	}
}

// removeFromNameIndex
void removeFromNameIndex(NameIndex* index, CString nameKey, CString recordKey) {
	NameIndexEl* el = (NameIndexEl*) searchHashTable(index, nameKey);
	if (!el) {
		// Log something happened.
		return;
	}
	Set* recordKeys = el->recordKeys;
	if (!isInSet(recordKeys, recordKey)) {
		// Log something happened.
		return;
	}
	removeFromSet(recordKeys, recordKey);
}

// Remove all names of a person from a NameIndex.
void removeNamesOfPersonFromIndex(NameIndex* index, GNode* person) {
	String recordKey = person->key;
	GNode* name = NAME(person);
	while (name) {
		String nameKey = nameToNameKey(name->value);
		removeFromNameIndex(index, nameKey, recordKey);
		name = name->sibling;
		if (name && nestr(name->tag, "NAME")) name = null;
	}
}

// searchNameIndex searches NameIndex for a name and returns the record keys that have the name.
// MNOTE: The set that is returned is in the NameIndex. It cannot be changed.
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
// MNOTE: nameKey is in static memory so must be saved.
// MNOTE: the Set is created to hold the record keys.
// MNOTE: the Set's delete function is null because the record keys are not freed.
static NameIndexEl* createNameIndexEl(CString nameKey) {
	NameIndexEl* el = (NameIndexEl*) stdalloc(sizeof(NameIndexEl));
	if (! el)
	  return NULL;
	memset (el, 0, sizeof(NameIndexEl));
	el->nameKey = strsave(nameKey);
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

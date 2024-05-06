// DeadEnds
//
// stringset.c
//
// Created by Thomas Wetmore on 20 April 2024.
// Last changed on 22 April 2024.

#include "stringset.h"

// getKey is the getKey function for StringSets.
static CString getKey(void* element) {
	return (CString) element;
}

// compare is the compare function for StringSets.
static int compare(CString element1, CString element2) { return strcmp(element1, element2); }

// createStringSet creates a StringSet.
StringSet* createStringSet(void) {
	return createSet(getKey, compare, null);
}

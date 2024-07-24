// DeadEnds
//
// refnindex.h is the hearder file for the reference (REFN) index features.
//
// Created on 17 December 2023.
// Last changed on 5 July 2024.

#ifndef refnindex_h
#define refnindex_h

#include "hashtable.h"

// RefnIndexEl is the type for elements in RefnIndexes. refn is the value of a 1 REFN Gedcom line
// and key is the tag of the record it is in. Each refn key must be unique.
typedef struct RefnIndexEl {
	CString refn;
	CString key;
} RefnIndexEl;

// RefnIndex is a HashTable holding RefnIndexEls.
typedef HashTable RefnIndex;

// Interface to RefnIndexes.
RefnIndexEl *createRefnIndexEl(CString refn, CString key);
RefnIndex *createRefnIndex(void);
void deleteRefnIndex(RefnIndex*);
bool addToRefnIndex(RefnIndex*, CString refn, CString key);
CString searchRefnIndex(RefnIndex*, CString refn);
void showRefnIndex(RefnIndex*);

#endif // refnindex_h

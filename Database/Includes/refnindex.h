//
//  DeadEnds Project
//
//  refnindex.h -- Header file for the reference (REFN) index features.
//
//  Created on 17 December 2023.
//  Last changed on 1 January 2024.
//

#ifndef refnindex_h
#define refnindex_h
#include "hashtable.h"

typedef struct RefnIndexEl {
	CString refn;  // Value of the REFN node.
	CString key;  // Key of the record with the REFN node.
} RefnIndexEl;

typedef HashTable RefnIndex;

RefnIndexEl *createRefnIndexEl(CString refn, CString key);
RefnIndex *createRefnIndex(void);
void deleteRefnIndex(RefnIndex*);
bool insertInRefnIndex(RefnIndex*, CString refn, CString key);
String searchRefnIndex(RefnIndex*, CString refn);
void showRefnIndex(RefnIndex*);

#endif // refnindex_h

//
//  DeadEnds Library
//
//  integertable.h is the header file for the IntegerTable type, a HashTable that maps Strings
//  to integers.
//
//  Created by Thomas Wetmore on 23 April 2023.
//  Last changed on 4 June 2025.
//

#ifndef integertable_h
#define integertable_h

#include "standard.h"

// IntegerTable is a HashTable that maps Strings to integers.
typedef struct HashTable HashTable;
typedef HashTable IntegerTable;

// IntegerElement is an element in an IntegerTable.
typedef struct IntegerElement {
    CString key;
    int value;
} IntegerElement;

// User interface.
extern CString integerGetKey(void* element);
IntegerTable *createIntegerTable(int numBuckets);
void insertInIntegerTable(IntegerTable*, CString key, int value);
int searchIntegerTable(IntegerTable*, CString key); // Returns NAN if key not there.
void incrIntegerTable (IntegerTable*, CString key);
void showIntegerTable(IntegerTable*); // For debugging.

#endif // integertable_h

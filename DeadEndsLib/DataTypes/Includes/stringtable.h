// DeadEnds Library
//
// stringtable.h is the header file for StringTables.
//
// Created by Thomas Wetmore on 23 April 2023.
// Last changed on 21 November 2024.

#ifndef stringtable_h
#define stringtable_h

#include "hashtable.h"

// StringTable is a HashTable that maps Strings to Strings.
typedef HashTable StringTable;

// StringElement is an element in a StringTable.
typedef struct StringElement {
	String key;
	String value;
} StringElement;

// User interface to string tables.
HashTable *createStringTable(int numBuckets);
String searchStringTable(StringTable*, CString key);
bool isInStringTable(StringTable *table, CString key);
void addToStringTable(StringTable*, CString key, CString value);
String fixString(StringTable *table, CString string);
void showStringTable(StringTable*);  //  For debugging.

#endif // stringtable_h

//
//  DeadEnds Library
//
//  stringtable.h is the header file for StringTables. A StringTable is a HashTable that maps Strings to String.
//
//  Created by Thomas Wetmore on 23 April 2023.
//  Last changed on 4 June 2025.
//

#ifndef stringtable_h
#define stringtable_h

typedef struct HashTable HashTable;
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
CString fixString(StringTable *table, CString string);
void showStringTable(StringTable*);  //  For debugging.

#endif // stringtable_h

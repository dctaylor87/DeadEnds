//
//  stringtable.h
//  JustParsing
//
//  Created by Thomas Wetmore on 23 April 2023.
//  Last changed on 25 September 2023.
//

#ifndef stringtable_h
#define stringtable_h

#include "hashtable.h"

//  StringTable -- Hash Table that maps strings to strings.
//--------------------------------------------------------------------------------------------------
typedef HashTable StringTable;

//  StringElement -- Elements for String to String mappings.
//--------------------------------------------------------------------------------------------------
typedef struct StringElement {
    String key;
    String value;
} StringElement;

//  User interface to string tables.
//--------------------------------------------------------------------------------------------------
HashTable *createStringTable(void);
String searchStringTable(StringTable*, CString key);
extern bool isInStringTable(StringTable *table, CString key);
void insertInStringTable(StringTable*, CString key, CString value);
String fixString(StringTable *table, String string);
void showStringTable(StringTable*);  //  For debugging.

#endif // stringtable_h

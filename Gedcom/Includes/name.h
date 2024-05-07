//
//  DeadEnds
//
//  name.h -- Header file for Gedcom name functions.
//
//  Created by Thomas Wetmore on 7 November 2022.
//  Last changed on 16 November 2023.
//

#ifndef name_h
#define name_h

typedef struct Database Database;
#include "standard.h"
#include "nameindex.h"

// Some functions use static dataspace to construct names. MAXNAMELEN is the maximum length.
//--------------------------------------------------------------------------------------------------
#define MAXNAMELEN 512

// Prototypes of functions defined in names.c.
//--------------------------------------------------------------------------------------------------
extern String upsurname(CString name); // Make a Gedcom name have an all uppercase surname.
String manipulateName(String, bool caps, bool reg, int maxlen);  // Manipulate a name.
String getSurname(CString);  // Get the surname of a Gedcom name.
String getGivenNames(CString);  // Get the given names of a Gedcom name.
int getFirstInitial(CString name);  // Get the first initial of a Gedcom name.
String soundex(CString surname);  // Get the Soundex code of a Gedcom surname.
extern bool remove_name (String name, String key); // Remove entry from name record.
String nameToNameKey(CString name);  // Convert a partial or full Gedcom name to a name key.
int compareNames(CString name1, CString name2); // Compare two Gedcom names.
String* personKeysFromName(CString name, Database*, int* pcount /*[, bool exact]*/);
String nameString(CString name);  // Remove slashes from a name.
String trimName (String name, int len);  // Trim name to specific length.
bool nameToList(String name, List*, int *len, int *sind);
extern bool exactMatch(String partial, String complete);

#endif // name_h

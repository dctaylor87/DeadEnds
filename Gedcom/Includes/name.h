//
//  DeadEnds
//
//  name.h -- Header file for Gedcom name functions.
//
//  Created by Thomas Wetmore on 7 November 2022.
//  Last changed on 22 August 2023.
//

#ifndef name_h
#define name_h

#include "standard.h"
#include "nameindex.h"

// Some functions use static dataspace to construct names. MAXNAMELEN is the maximum length.
//--------------------------------------------------------------------------------------------------
#define MAXNAMELEN 512

// Prototypes of functions defined in names.c.
//--------------------------------------------------------------------------------------------------
extern String upsurname(String name); // Make a Gedcom name have an all uppercase surname.
String manipulateName(String, bool caps, bool reg, int maxlen);  // Manipulate a name.
String getSurname(String);  // Get the surname of a Gedcom name.
String getGivenNames(String);  // Get the given names of a Gedcom name.
int getFirstInitial(String name);  // Get the first initial of a Gedcom name.
String soundex(CString surname);  // Get the Soundex code of a Gedcom surname.
extern bool remove_name (String name, String key); // Remove entry from name record.
String nameToNameKey(String name);  // Convert a partial or full Gedcom name to a name key.
int compareNames(String name1, String name2); // Compare two Gedcom names.
String* personKeysFromName(String name, NameIndex*, int* pcount /*[, bool exact]*/);
String nameString(String name);  // Remove slashes from a name.
String trimName (String name, int len);  // Trim name to specific length.
extern bool exactMatch(String partial, String complete);

#endif // name_h

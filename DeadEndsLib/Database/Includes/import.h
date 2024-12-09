// DeadEnds
//
// import.h has the declarations of the high level import function.
// TODO: The string functions aren't here yet.
//
// Created by Thomas Wetmore on 13 November 2022.
// Last changed on 6 December 2024.

#ifndef import_h
#define import_h

#include <stdio.h>
#include "recordindex.h"
#include "errors.h"
#include "database.h"
#include "gnodelist.h"
#include "stringset.h"
#include "integertable.h"

/* if the ImportFeedback* argument is non-NULL, then the if_* argument
   is called (provided it is non-NULL) at various times during the
   import to provide feedback to the user.  */

struct ImportFeedback
{
  void (*if_validation_warning_fnc)(CString msg);
  void (*if_validation_error_fnc)(CString msg);
  void (*if_error_invalid_fnc)(CString reason);
  void (*if_validating_fnc)(void);
  void (*if_validated_rec_fnc)(RecordType type, CString tag, int count);
  void (*if_beginning_import_fnc)(CString msg);
  void (*if_error_readonly_fnc)(void);
  void (*if_adding_unused_keys_fnc)(void);
  void (*if_added_rec_fnc)(RecordType type, CString tag, int count);
};

typedef struct ImportFeedback ImportFeedback;

List *getDatabasesFromFiles(List*, int vcodes, ErrorLog*);
Database* getDatabaseFromFile(CString, int vcodes, ErrorLog*);
RecordIndex* getRecordIndexFromFile(String, RootList*, RootList*, IntegerTable*, ErrorLog*);
void checkKeysAndReferences(GNodeList*, String name, IntegerTable*, ErrorLog*);
extern Database *importFromFileFP(File* file, CString path, ErrorLog *log);

#endif // import_h

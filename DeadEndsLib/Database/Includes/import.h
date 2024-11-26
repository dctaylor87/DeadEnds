// DeadEnds
//
// import.h -- Header file for the Gedcom import process.
//
// Created by Thomas Wetmore on 13 November 2022.
// Last changed on 19 November 2024.

#ifndef import_h
#define import_h

#include <stdio.h>
#include "recordindex.h"
#include "errors.h"
#include "database.h"
#include "gnodelist.h"
#include "stringset.h"

List *gedcomFilesToDatabases(List*, ErrorLog*);
Database *importFromFile(String, ErrorLog*);
Database* gedcomFileToDatabase(CString, ErrorLog*);
void checkKeysAndReferences(GNodeList*, String name, IntegerTable*, ErrorLog*);
extern Database *importFromFileFP(File* file, CString path, ErrorLog *log);

#endif // import_h

// DeadEnds
//
// import.h -- Header file for the Gedcom import process.
//
// Created by Thomas Wetmore on 13 November 2022.
// Last changed on 21 July 2024.

#ifndef import_h
#define import_h

#include <stdio.h>
#include "recordindex.h"
#include "errors.h"
#include "database.h"
#include "gnodelist.h"
#include "stringset.h"

List *importFromFiles(String fileNames[], int count, ErrorLog*);
Database *importFromFile(String, ErrorLog*);
Database* gedcomFileToDatabase(CString, ErrorLog*);
void checkKeysAndReferences(GNodeList*, String name, ErrorLog*);
extern Database *importFromFileFP(File* file, CString path, ErrorLog *log);

#endif // import_h

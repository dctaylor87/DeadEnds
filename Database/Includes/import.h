//
//  DeadEnds
//
//  import.h -- Header file for the Gedcom import process.
//
//  Created by Thomas Wetmore on 13 November 2022.
//  Last changed on 5 August 2023.
//

#ifndef import_h
#define import_h

#include <stdio.h>
#include "recordindex.h"
#include "errors.h"
#include "database.h"

bool importFromFiles(String fileNames[], int count, ErrorLog*);
extern Database *simpleImportFromFile(FILE* file, ErrorLog *errorLog);

#endif // import_h

//
//  DeadEnds
//
//  parse.h -- Header file for the user interface to parsing.
//
//  Created by Thomas Wetmore on 4 January 23.
//  Last changed on 19 January 2023.
//

#ifndef parse_h
#define parse_h

#include "standard.h"  // String.
#include "errors.h"

// Parse fileName found in searchPath.
void parseProgram(CString fileName, CString searchPath, ErrorLog *errorLog);
extern CString curFileName;  // Name of file being parsed.
extern int curLine;   // Current line number in file being parsed.
extern FILE *currentFile;

#endif // parse_h

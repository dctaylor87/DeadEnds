//
//  DeadEnds
//
//  parse.h -- Header file for the user interface to parsing.
//
//  Created by Thomas Wetmore on 4 January 23.
//  Last changed on 17 August 2025.
//

#ifndef parse_h
#define parse_h

#include "standard.h"  // String.
#include "errors.h"
typedef struct Program Program;

// Parse fileName found in searchPath.
Program* parseProgram(CString, CString, ErrorLog *errorLog);  // Parse script program.
extern CString curFileName;  // Name of file being parsed.
extern int curLine;   // Current line number in file being parsed.
extern FILE *currentFile;
extern List* pendingFiles; // Files to be parsed.

#endif // parse_h

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

void parseProgram(CString fileName, CString searchPath);// Parse fileName found in searchPath.
extern CString currentProgramFileName;			// Name of file being parsed.
extern int currentProgramLineNumber;			// Current line number in file being parsed.
extern FILE *currentProgramFile;

#endif // parse_h

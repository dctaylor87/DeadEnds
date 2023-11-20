//
//  DeadEnds
//
//  readnode.h -- Header file for routines and variables that read Gedcom files.
//
//  Created by Thomas Wetmore on 17 December 2022.
//  Last changed on 19 November 2023.
//

#ifndef readnode_h
#define readnode_h

#include "standard.h"
#include "gnode.h"
#include "errors.h"

// Return codes used by functions that extract Gedcom nodes from Gedcom data.
//--------------------------------------------------------------------------------------------------
#define OKAY  1
#define ERROR 0
#define DONE -1

// Current line number in file being read.
//extern int fileLine;  // Current line number in file being read.

GNode* firstNodeTreeFromFile(FILE*, int *line, ErrorLog*);
GNode* nextNodeTreeFromFile(FILE*, int *line, ErrorLog*);

#endif

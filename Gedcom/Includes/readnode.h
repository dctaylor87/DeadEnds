//
//  DeadEnds
//
//  readnode.h -- Header file for routines and variables that read Gedcom files.
//
//  Created by Thomas Wetmore on 17 December 2022.
//  Last changed on 26 April 2023.
//

#ifndef readnode_h
#define readnode_h

#include "standard.h"
#include "gnode.h"

// Return codes used by the functions that extract Gedcom nodes from Gedcom files.
//--------------------------------------------------------------------------------------------------
#define OKAY  1
#define ERROR 0
#define DONE -1

// Global variables defined in readnode.c
extern int fileLine;  // Current line number in file being read.

//static bool stringToLine(String*, int*, String*, String*, String*);
//static bufferToLine(String, int*, String*, String*, String*, String*)
int fileToLine(FILE *fp, int *plev, String *pxref, String *ptag, String *pval, String *pmsg);

#endif /* readnode_h */

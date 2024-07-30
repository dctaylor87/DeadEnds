// DeadEnds
//
// path.h
//
// Created by Thomas Wetmore on 14 December 2022.
// Last changed 26 July 2024.

#ifndef path_h
#define path_h

#include <stdio.h>
#include "standard.h"

#define MAXPATHLENGTH 1024

FILE *fopenPath(CString fileName, CString mode, CString searchPath);
CString resolveFile(CString name, CString path);
String lastPathSegment(CString fileName);

#endif // path_h

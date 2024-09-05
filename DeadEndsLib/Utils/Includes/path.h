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

#if defined WIN32 && !defined __CYGWIN__

#define DEREADTEXT "rt"
#define DEREADBINARY "rb"
#define DEREADBINARYUPDATE "r+b"
#define DEWRITETEXT "wt"
#define DEWRITEBINARY "wb"
#define DEAPPENDTEXT "at"

#define DEFILERANDOM "R"
#define DEFILETEMP "T"

#define DESTRPATHSEPARATOR ";"
#define DESTRDIRSEPARATOR "\\"
#define DECHRPATHSEPARATOR ';'
#define DECHRDIRSEPARATOR '\\'

#else

#define DEREADTEXT "r"
#define DEREADBINARY "r"
#define DEREADBINARYUPDATE "r+"
#define DEWRITETEXT "w"
#define DEWRITEBINARY "w"
#define DEAPPENDTEXT "a"

#define DEFILERANDOM ""
#define DEFILETEMP ""

#define DESTRPATHSEPARATOR ":"
#define DESTRDIRSEPARATOR "/"
#define DECHRPATHSEPARATOR ':'
#define DECHRDIRSEPARATOR '/'

#endif

FILE *fopenPath(CString fileName, CString mode, CString searchPath);
CString resolveFile(CString fileName, CString searchPath);
String lastPathSegment(CString fileName);
bool pathMatch (CString path1, CString path2);
String pathConcatAllocate (CString dir, CString file);
String pathConcat (CString dir, CString file, int utf8, String buffer, int buflen);
bool isDirSep (char c);


#endif // path_h

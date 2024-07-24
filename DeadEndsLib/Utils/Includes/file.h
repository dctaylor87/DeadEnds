// DeadEnds
//
// file.h
//
// Created by Thomas Wetmore on 1 July 2024.
// Last changed on 21 July 2024.

#ifndef file_h
#define file_h

#include "standard.h"

// File is a structure that holds a file's name and Unix FILE pointer.
typedef struct File {
	FILE* fp;
	String path;
	String name;
} File;

File* openFile(CString path, CString mode);
void closeFile(File*);

#endif // file_h

//
//  path.h
//  ImportGedcom
//
//  Created by Thomas Wetmore on 14 December 2022.
//  Last changed 20 January 2023.
//

#ifndef path_h
#define path_h

// fopenpath -- Open a file using a search path.
FILE *fopenpath(CString fileName, CString mode, CString searchPath);

#endif /* path_h */

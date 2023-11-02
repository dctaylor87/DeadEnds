//
//  date.h
//  JustParsing
//
//  Created by Thomas Wetmore on 22 February 2023.
//  Last changed on 29 September 2023.
//

#ifndef date_h
#define date_h

#include <stdio.h>
#include "standard.h"

String format_date (CString str, int dfmt, int mfmt, int yfmt, int sfmt, bool cmplx);
void extract_date(CString, int*, int*, int*, int*, String*);
extern String get_date(void);

#endif /* date_h */

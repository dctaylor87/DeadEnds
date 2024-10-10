// DeadEnds
//
// date.h is the header file for the functions that manipulate Gedcom date Strings.
//
// Created by Thomas Wetmore on 22 February 2023.
// Last changed on 19 August 2024.

#ifndef date_h
#define date_h

#include <stdio.h>
#include "standard.h"

typedef enum DateToken {
	monthToken = 260,
	wordToken,
	charToken,
	intToken,
	unknownToken,
	atEndToken = 0
} DateToken;

/* static */ void setExtractString(CString);
/*static*/ DateToken getDateToken(int *pInt, String *pString);

String formatDate (CString str, int dfmt, int mfmt, int yfmt, int sfmt, bool cmplx);
void extractDate(CString, int*, int*, int*, int*, String*);
extern String get_date(void);

#endif // date_h

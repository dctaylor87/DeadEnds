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

/* static */ void setExtractString(String);
/*static*/ DateToken getDateToken(int *pInt, String *pString);

void extractDate(String, int*, int*, int*, int*, String*);

#endif // date_h

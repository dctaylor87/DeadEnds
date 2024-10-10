// DeadEnds
//
// gedcom.c
//
// Created by Thomas Wetmore on 29 November 2022.
// Last changed on 30 May 2024.

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "refnindex.h"
#include "gedcom.h"

// recordType returns the type of a Gedcom record.
RecordType recordType(GNode* root) {
    ASSERT(root);
    String tag = root->tag;
	//printf("%s ", tag);
    if (eqstr(tag, "INDI")) return GRPerson;
    if (eqstr(tag, "FAM"))  return GRFamily;
    if (eqstr(tag, "SOUR")) return GRSource;
    if (eqstr(tag, "EVEN")) return GREvent;
    if (eqstr(tag, "HEAD")) return GRHeader;
    if (eqstr(tag, "TRLR")) return GRTrailer;
    return GROther;
}

//  compareRecordKeys compares record keys; longer keys sort after shorter keys.
int compareRecordKeys(CString a, CString b) {
    ASSERT(strlen(a) > 1 && strlen(b) > 1);  // Is this strictly necessary?
    if (strlen(a) != strlen(b)) return (int) (strlen(a) - strlen(b));
    for (int i = 0; i < (int)strlen(a); i++) {
        if (a[i] != b[i]) return a[i] - b[i];
    }
    return 0;
}

// sexTypeToString returns the Gedcom character for a SexType.
String sexTypeToString(SexType sex) {
	if (sex == sexMale) return "M";
	if (sex == sexFemale) return "F";
	if (sex == sexUnknown) return "U";
	return "X";
}

//
SexType sexTypeFromString(CString s) {
	if (eqstr("M", s)) return sexMale;
	if (eqstr("F", s)) return sexFemale;
	if (eqstr("U", s)) return sexUnknown;
	return sexError;
}

bool validSexString(String s) {
	if (!s) return false;
	return sexTypeFromString(s) != sexError;
}

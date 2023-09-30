//
//  DeadEnds
//
//  gedcom.h
//
//  Created by Thomas Wetmore on 7 November 2022.
//  Last changed on 9 September 2023.
//

#ifndef gedcom_h
#define gedcom_h

typedef struct g GNode;  //  Forward reference.
#include "standard.h"
#include "gnode.h"

//  SexType -- Enumeration for sex types.
//--------------------------------------------------------------------------------------------------
typedef enum st {
    sexMale = 1, sexFemale, sexUnknown
} SexType;

#define personToKey(indi) rmvat(indi->key)
#define familyToKey(fam)  rmvat(fam->key)

//  RecordType -- Enumeration of supported DeadEnds record types.
//--------------------------------------------------------------------------------------------------
typedef enum {
    GRUnknown = 0, GRPerson, GRFamily, GRSource, GREvent, GROther, GRHeader, GRTrailer
} RecordType;

RecordType recordType(GNode *root);  // Return the type of a Gedcom record tree.

int compareRecordKeys(Word p, Word q);  // gedcom.c

// FORCHILDREN / ENDCHILDREN -- Iterator for the children of a family.
//--------------------------------------------------------------------------------------------------
#define FORCHILDREN(fam, childd, num) \
    {\
    GNode* __node = findTag(fam->child, "CHIL");\
    GNode* childd;\
    int num = 0;\
    while (__node) {\
        childd = keyToPerson(rmvat(__node->value), theIndex);\
        ASSERT(childd);\
        num++;\
        {

#define ENDCHILDREN \
        }\
        __node = __node->sibling;\
        if (__node && nestr(__node->tag, "CHIL")) __node = null;\
    }}

//  FORFAMCS / ENDFAMCS
#define FORFAMILIES(indi, fam, num)\
{\
    GNode *__node = FAMS(indi);\
    GNode *fam;\
    int num = 0;\
    while (__node) {\
        fam = keyToFamily(rmvat(__node->value), theIndex);\
        ASSERT(fam);\
        {
#define ENDFAMILIES\
        }\
        __node = __node->sibling;\
        if (__node && nestr(__node->tag, "FAMS")) __node = null;\
    }\
}

//  FORFAMSS / ENDFAMSS -- Iterator for a person's families as a spouse.
//--------------------------------------------------------------------------------------------------
#define FORFAMSS(indi, fam, spouse, num)\
    {\
    GNode* __node = FAMS(indi);\
    int __sex = SEXV(indi);\
    GNode *fam, *spouse;\
    int num = 0;\
    while (__node) {\
        fam = keyToFamily(rmvat(__node->value), theIndex);\
        ASSERT(fam);\
        if (__sex == sexMale)\
            spouse = familyToWife(fam);\
        else\
            spouse = familyToHusband(fam);\
        num++;\
        {

#define ENDFAMSS \
        }\
        __node = __node->sibling;\
        if (__node && nestr(__node->tag, "FAMS")) __node = null;\
    }}

// FORFAMCS / ENDFAMCS -- Iterator for a person's families as a child.
//--------------------------------------------------------------------------------------------------
#define FORFAMCS(indi, fam, fath, moth, num)\
{\
    GNode* __node = FAMC(indi);\
    GNode *fam, *fath, *moth;\
    int num = 0;\
    while (__node) {\
        fam = keyToFamily(rmvat(__node->value), theIndex);\
        ASSERT(fam);\
        fath = familyToHusband(fam);\
        moth = familyToWife(fam);\
        num++;\
        {
#define ENDFAMCS\
        }\
        __node = __node->sibling;\
        if (__node && nestr(__node->tag, "FAMC")) __node = null;\
    }\
}

//  FORTAGVALUES -- Iterate a list of nodes looking for a particular tag.
//--------------------------------------------------------------------------------------------------
#define FORTAGVALUES(root, tagg, node, value)\
{\
    GNode *node, *__node = root->child;\
    String value, __value;\
    while (__node) {\
        while (__node && strcmp(tagg, __node->tag))\
            __node = __node->sibling;\
        if (__node == null) break;\
        __value = value = full_value(__node);\
        node = __node;\
        {
#define ENDTAGVALUES\
        }\
        if (__value) stdfree(__value);\
        __node = __node->sibling;\
    }\
}

//  FORHUSBS -- Iterate over the husbands in one family; handles non-traditional families.
//--------------------------------------------------------------------------------------------------
#define FORHUSBS(fam, husb, num)\
{\
    GNode* __node = findTag(fam->child, "HUSB");\
    GNode* husb=0;\
    String __key=0;\
    int num = 0;\
    while (__node) {\
        __key = rmvat(__node->value);\
        if (!__key || !(husb = keyToPerson(__key, theIndex))) {\
            ++num;\
            __node = __node->sibling;\
            continue;\
        }\
        ++num;\
        {
#define ENDHUSBS\
        }\
        __node = __node->sibling;\
        if (__node && nestr(__node->tag, "HUSB")) __node = null;\
    }\
}

//  FORWIFES -- Iterate over the wives in one family; handles non-traditional families.
//--------------------------------------------------------------------------------------------------
#define FORWIFES(fam, wife, num)\
{\
    GNode* __node = findTag(fam->child, "WIFE");\
    GNode* wife = null;\
    String __key = null;\
    int num = 0;\
    while (__node) {\
        __key = rmvat(__node->value);\
        if (!__key || !(wife = keyToPerson(__key, theIndex))) {\
            ++num;\
            __node = __node->sibling;\
            if (__node && nestr(__node->tag, "WIFE")) __node = null;\
                continue;\
        }\
        ASSERT(wife);\
        num++;\
        {
#define ENDWIFES\
        }\
        __node = __node->sibling;\
        if (__node && nestr(__node->tag, "WIFE")) __node = null;\
    }\
}

//  FORSPOUSES -- Iterate over a person's spouses.
//--------------------------------------------------------------------------------------------------
#define FORSPOUSES(indi, spouse, fam, num)\
{\
    GNode* __fnode = FAMS(indi);\
    int __sex = SEXV(indi);\
    GNode* spouse;\
    GNode* fam;\
    int num = 0;\
    while (__fnode) {\
        spouse = null;\
        fam = keyToFamily(rmvat(__fnode->value), theIndex);\
        if (__sex == sexMale)\
            spouse = familyToWife(fam);\
        else\
            spouse = familyToHusband(fam);\
            if (spouse != null) {\
            num++;\
            {
#define ENDSPOUSES\
            }\
        }\
        __fnode = __fnode->sibling;\
        if(__fnode && nestr("FAMS", __fnode->tag)) __fnode = null;\
    }\
}

//  Macros that return specific gedcom nodes from a record tree.
//--------------------------------------------------------------------------------------------------
#define NAME(indi)  findTag(indi->child,"NAME")
#define SEX(indi)   findTag(indi->child,"SEX")
#define SEXV(indi)  val_to_sex(findTag(indi->child,"SEX"))
#define BIRT(indi)  findTag(indi->child,"BIRT")
#define DEAT(indi)  findTag(indi->child,"DEAT")
#define BAPT(indi)  findTag(indi->child,"CHR")
#define BURI(indi)  findTag(indi->child,"BURI")
#define FAMC(indi)  findTag(indi->child,"FAMC")
#define FAMS(indi)  findTag(indi->child,"FAMS")
#define HUSB(fam)   findTag(fam->child,"HUSB")
#define WIFE(fam)   findTag(fam->child,"WIFE")
#define MARR(fam)   findTag(fam->child,"MARR")
#define CHIL(fam)   findTag(fam->child,"CHIL")
#define DATE(evnt)  findTag(evnt->child,"DATE")
#define PLAC(evnt)  findTag(evnt->child,"PLAC")

#endif // gedcom_h

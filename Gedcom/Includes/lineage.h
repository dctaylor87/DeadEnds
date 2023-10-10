//
//  DeadEnds
//
//  lineage.h -- Header file for operations on Gedcom nodes based on genealogical relationsips
//    and properties
//
//  Created by Thomas Wetmore on 17 February 2023.
//  Last changed on 10 October 2023.
//

#ifndef lineage_h
#define lineage_h

#include "gnode.h"

GNode* personToFather(GNode*);  //  Return first father of a person.
GNode* personToMother(GNode*);  //  Return first wife of a person.
GNode* personToPreviousSibling(GNode* indi);  //  Return the previous sibling of a person.
GNode* personToNextSibling(GNode* indi);  //  Return the next sibling of a person.
GNode* familyToHusband(GNode* fam);  //  Return the first husband of a family.
GNode* familyToWife(GNode* fam);  //  Return the first wife of a family.
GNode* familyToFirstChild(GNode* fam);  //  Return the first child of a family.
GNode* familyToLastChild(GNode* fam);  //  Return the last child of a family.
String personToName(GNode*, int);  //  Return the first name of a person.
String personToTitle(GNode*, int);  //  Return the first title of a person.
int numberOfSpouses(GNode* indi);  //  Return the number of spouses of a person.


GNode* fam_to_spouse(Database*, GNode*, GNode*);

#endif // lineage_h

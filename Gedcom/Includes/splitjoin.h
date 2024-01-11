//
//  DeadEnds
//
//  splitjoin.h
//
//  Created by Thomas Wetmore on 7 November 2022.
//  Last changed on 7 August 2023.
//

#ifndef splitjoin_h
#define splitjoin_h

#include "gnode.h"

// Prototypes of the functions found in splitjoin.c
//--------------------------------------------------------------------------------------------------
void splitPerson(GNode*, GNode**, GNode**, GNode**, GNode**, GNode**, GNode**);
void joinPerson(GNode*, GNode*, GNode*, GNode*, GNode*, GNode*, GNode*);
void splitFamily(GNode*, GNode**, GNode**, GNode**, GNode**, GNode**);
void joinFamily(GNode*, GNode*, GNode*, GNode*, GNode*, GNode*);

void splitSource(GNode *root, GNode **prefn, GNode **pbody);
void joinSource (GNode *root, GNode *refn, GNode *body);
void splitEvent(GNode *root, GNode **prefn, GNode **pbody);
void joinEvent (GNode *root, GNode *refn, GNode *body);
void splitOther(GNode *root, GNode **prefn, GNode **pbody);
void joinOther (GNode *root, GNode *refn, GNode *body);

GNode *normalizePerson(GNode*);
GNode *normalizeFamily(GNode*);
GNode *normalizeEvent(GNode*);
GNode *normalizeSource(GNode*);
GNode *normalizeOther(GNode*);

#endif // splitjoin_h

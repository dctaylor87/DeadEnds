//
//  DeadEnds
//
//  splitjoin.c -- Functions that split persons and families into parts and join them back
//    together. Calling split and then join formats person and family node trees into
//    canonical order. This can make followon operations more efficient.
//
//  Created by Thomas Wetmore on 7 November 2022.
//  Last changed on 19 December 2023.
//

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */

#include "standard.h"
#include "gnode.h"
#include "splitjoin.h"

//  splitPerson -- Split a person Node tree into parts. The Nodes are not copied; they are
//    separated into different lists.
//--------------------------------------------------------------------------------------------------
void splitPerson(GNode *indi, GNode **pname, GNode **prefn, GNode **psex, GNode **pbody,
                 GNode **pfamc, GNode **pfams)
// indi -- (in) Root of person node tree to split.
// pname -- (out) Pointer to list of 1 NAME Nodes.
// prefn -- (out) Pointer to list of 1 REFN Nodes.
// psex -- (out) Pointer to first 1 SEX Node (others are in body).
// pbody -- (out) Pointer to other level 1 Nodes.
// pfamc -- (out) Pointer to list of 1 FAMC Nodes.
// pfams -- (out) Pointer to list of 1 FAMS Nodes.
{
    GNode *name, *lnam, *refn, *sex, *body, *famc, *fams, *last;
    GNode *lfmc, *lfms, *lref, *prev, *node;
    ASSERT(eqstr("INDI", indi->tag));
    name = sex = body = famc = fams = last = lfms = lfmc = lnam = null;
    refn = lref = null;
    node = indi->child;  // Prepare to iterate through the children of the INDI Node.
    indi->child = indi->sibling = null;  // Disconnect the root from its child and sibling.
    // Worry. Does disconnecting from a sibling cause any future issues?
    while (node) {  // Iterate through the children of the INDI Node.
        String tag = node->tag;
        if (eqstr("NAME", tag)) {  // Handle 1 NAME Nodes.
            if (!name) name = lnam = node;
            else lnam = lnam->sibling = node;
        } else if (!sex && eqstr("SEX", tag)) {  // Handle the first 1 SEX Node.
            sex = node;
        } else if (eqstr("FAMC", tag)) {  // Handle the 1 FAMC Nodes.
            if (!famc) famc = lfmc = node;
            else lfmc = lfmc->sibling = node;
         } else if (eqstr("FAMS", tag)) {  // Handle the 1 FAMS Nodes.
            if (!fams) fams = lfms = node;
            else lfms = lfms->sibling = node;
         } else if (eqstr("REFN", tag)) {  // Handle the 1 REFN Nodes.
            if (!refn) refn = lref = node;
            else lref = lref->sibling = node;
        } else {  // Handle all other level 1 Nodes.
            if (!body) body = last = node;
            else last = last->sibling = node;
        }
        prev = node;
        node = node->sibling;
        prev->sibling = null;
    }
    *pname = name;
    *prefn = refn;
    *psex = sex;
    *pbody = body;
    *pfamc = famc;
    *pfams = fams;
}

//  joinPerson -- Join a person node tree from parts. This neither allocates nor frees any nodes.
//--------------------------------------------------------------------------------------------------
void joinPerson (GNode *indi, GNode *name, GNode *refn, GNode *sex, GNode *body, GNode *famc,
                 GNode *fams)
//  indi, name, refn, sex, body, famc, fams;
{
    GNode *node = null;
    ASSERT(indi && eqstr("INDI", indi->tag));

    indi->child = null;
    if (name) {
        indi->child = node = name;
        while (node->sibling)
            node = node->sibling;
    }
    if (refn) {
        if (node) node = node->sibling = refn;
        else indi->child = node = refn;
        while (node->sibling) node = node->sibling;
    }
    if (sex) {
        if (node) node = node->sibling = sex;
        else indi->child = node = sex;
    }
    if (body) {
        if (node) node = node->sibling = body;
        else indi->child = node = body;
        while (node->sibling) node = node->sibling;
    }
    if (famc) {
        if (node) node = node->sibling = famc;
        else indi->child = node = famc;
        while (node->sibling) node = node->sibling;
    }
    if (fams) {
        if (node) node->sibling = fams;
        else indi->child = fams;
    }
}

//  splitFamily -- Split a family into parts. This neither allocates nor frees nodes.
//--------------------------------------------------------------------------------------------------
void splitFamily(GNode* fam, GNode** prefn, GNode** phusb, GNode** pwife, GNode** pchil,
                  GNode** prest)
//  fam, prefn, phusb, pwife, pchil, prest
{
    GNode *node, *rest, *last, *husb, *lhsb, *wife, *lwfe, *chil, *lchl;
    GNode *prev, *refn, *lref;
    String tag;

    rest = last = husb = wife = chil = lchl = lhsb = lwfe = null;
    prev = refn = lref = null;
    node = fam->child;
    fam->child = fam->sibling = null;
    while (node) {
        tag = node->tag;
        if (eqstr("HUSB", tag)) {
            if (husb)
                lhsb = lhsb->sibling = node;
            else
                husb = lhsb = node;
        } else if (eqstr("WIFE", tag)) {
            if (wife)
                lwfe = lwfe->sibling = node;
            else
                wife = lwfe = node;
        } else if (eqstr("CHIL", tag)) {
            if (chil)
                lchl = lchl->sibling = node;
            else
                chil = lchl = node;
        } else if (eqstr("REFN", tag)) {
            if (refn)
                lref = lref->sibling = node;
            else
                refn = lref = node;
        } else if (rest)
            last = last->sibling = node;
        else
            last = rest = node;
        prev = node;
        node = node->sibling;
        prev->sibling = null;
    }
    *prefn = refn;
    *phusb = husb;
    *pwife = wife;
    *pchil = chil;
    *prest = rest;
}

//  joinFamily -- Join family from parts.
//--------------------------------------------------------------------------------------------------
void joinFamily (GNode *fam, GNode *refn, GNode *husb, GNode *wife, GNode *chil, GNode *rest)
// Node fam, refn, husb, wife, chil, rest;
{
    GNode *node = null;

    fam->child = null;
    if (refn) {
        fam->child = node = refn;
        while (node->sibling) node = node->sibling;
    }
    if (husb) {
        if (node) node = node->sibling = husb;
        else fam->child = node = husb;
        while (node->sibling) node = node->sibling;
    }
    if (wife) {
        if (node) node = node->sibling = wife;
        else fam->child = node = wife;
        while (node->sibling) node = node->sibling;
    }
    if (chil) {
        if (node) node = node->sibling = chil;
        else fam->child = node = chil;
		while (node->sibling) node = node->sibling;
    }
	if (rest) {
		if (node) node = node->sibling = rest;
		else fam->child = node = rest;
		while (node->sibling) node = node->sibling;
	}
}

//  splitSource -- Split a Source Node tree into parts. The Nodes are not copied; they are
//    separated into different lists.
//--------------------------------------------------------------------------------------------------
void splitSource(GNode *root, GNode **prefn, GNode **pbody)
// root  -- (in) Root of Source node tree to split.
// prefn -- (out) Pointer to list of 1 REFN Nodes.
// pbody -- (out) Pointer to other level 1 Nodes.
{
  GNode *refn = null;
  GNode *body = null;
  GNode *last = null;
  GNode *lref = null;
  GNode *prev = null;
  GNode *node = null;

  ASSERT(eqstr("SOUR", root->tag));
  ASSERT(! (root->sibling));	// root nodes should *NEVER* have siblings!

  node = root->child;  // Prepare to iterate through the children of the INDI Node.
  root->child = null;  // Disconnect the root from its child

  while (node) {  // Iterate through the children of the SOUR Node.
    String tag = node->tag;

    if (eqstr("REFN", tag)) {  // Handle the 1 REFN Nodes.
      if (!refn)
	refn = lref = node;
      else
	lref = lref->sibling = node;
    } else {  // Handle all other level 1 Nodes.
      if (!body)
	body = last = node;
      else
	last = last->sibling = node;
    }
    prev = node;
    node = node->sibling;
    prev->sibling = null;
  }
  *prefn = refn;
  *pbody = body;
}

//  joinSource -- Join a Source node tree from parts. This neither allocates nor frees any nodes.
//--------------------------------------------------------------------------------------------------
void joinSource (GNode *root, GNode *refn, GNode *body)
//  source, refn;
{
  GNode *node = null;
  ASSERT(root && eqstr("SOUR", root->tag));

  root->child = null;
  if (refn) {
    root->child = node = refn;

    while (node->sibling)
      node = node->sibling;
  }
  if (body) {
    if (node)
      node = node->sibling = body;
    else
      root->child = node = body;
  }
}

//  splitEvent -- Split a Event Node tree into parts. The Nodes are not copied; they are
//    separated into different lists.
//--------------------------------------------------------------------------------------------------
void splitEvent(GNode *root, GNode **prefn, GNode **pbody)
// root  -- (in) Root of Event node tree to split.
// prefn -- (out) Pointer to list of 1 REFN Nodes.
// pbody -- (out) Pointer to other level 1 Nodes.
{
  GNode *refn = null;
  GNode *body = null;
  GNode *last = null;
  GNode *lref = null;
  GNode *prev = null;
  GNode *node = null;

  ASSERT(eqstr("EVEN", root->tag));
  ASSERT(! (root->sibling));	// root nodes should *NEVER* have siblings!

  node = root->child;  // Prepare to iterate through the children of the EVEN Node.
  root->child = null;  // Disconnect the root from its child

  while (node) {  // Iterate through the children of the SOUR Node.
    String tag = node->tag;

    if (eqstr("REFN", tag)) {  // Handle the 1 REFN Nodes.
      if (!refn)
	refn = lref = node;
      else
	lref = lref->sibling = node;
    } else {  // Handle all other level 1 Nodes.
      if (!body)
	body = last = node;
      else
	last = last->sibling = node;
    }
    prev = node;
    node = node->sibling;
    prev->sibling = null;
  }
  *prefn = refn;
  *pbody = body;
}

//  joinEvent -- Join an Event node tree from parts. This neither allocates nor frees any nodes.
//--------------------------------------------------------------------------------------------------
void joinEvent (GNode *root, GNode *refn, GNode *body)
//  event, refn;
{
  GNode *node = null;
  ASSERT(root && eqstr("EVEN", root->tag));

  root->child = null;
  if (refn) {
    root->child = node = refn;

    while (node->sibling)
      node = node->sibling;
  }
  if (body) {
    if (node)
      node = node->sibling = body;
    else
      root->child = node = body;
  }
}

//  splitOther -- Split an Other Node tree into parts. The Nodes are not copied; they are
//    separated into different lists.
//--------------------------------------------------------------------------------------------------
void splitOther(GNode *root, GNode **prefn, GNode **pbody)
// root  -- (in) Root of Other node tree to split.
// prefn -- (out) Pointer to list of 1 REFN Nodes.
// pbody -- (out) Pointer to other level 1 Nodes.
{
  GNode *refn = null;
  GNode *body = null;
  GNode *last = null;
  GNode *lref = null;
  GNode *prev = null;
  GNode *node = null;

  ASSERT(root);
  ASSERT(! (root->sibling));	// root nodes should *NEVER* have siblings!

  node = root->child;  // Prepare to iterate through the children of the other Node.
  root->child = null;  // Disconnect the root from its child

  while (node) {  // Iterate through the children of the SOUR Node.
    String tag = node->tag;

    if (eqstr("REFN", tag)) {  // Handle the 1 REFN Nodes.
      if (!refn)
	refn = lref = node;
      else
	lref = lref->sibling = node;
    } else {  // Handle all other level 1 Nodes.
      if (!body)
	body = last = node;
      else
	last = last->sibling = node;
    }
    prev = node;
    node = node->sibling;
    prev->sibling = null;
  }
  *prefn = refn;
  *pbody = body;
}

//  joinOther -- Join an Other node tree from parts. This neither allocates nor frees any nodes.
//--------------------------------------------------------------------------------------------------
void joinOther (GNode *root, GNode *refn, GNode *body)
//  other, refn;
{
  GNode *node = null;
  ASSERT(root);

  root->child = null;
  if (refn) {
    root->child = node = refn;

    while (node->sibling)
      node = node->sibling;
  }
  if (body) {
    if (node)
      node = node->sibling = body;
    else
      root->child = node = body;
  }
}

//  normalizePerson - Get a person gedcom tree into standard format.
//--------------------------------------------------------------------------------------------------
GNode *normalizePerson(GNode *indi)
{
    GNode *names, *refns, *sex, *body, *famcs, *famss;
    splitPerson(indi , &names, &refns, &sex, &body, &famcs, &famss);
    joinPerson(indi, names, refns, sex, body, famcs, famss);
    return indi;
}

//  normalizeFamily -- Get a family gedcom tree into standard format.
//--------------------------------------------------------------------------------------------------
GNode *normalizeFamily(GNode *fam)
{
    GNode *refns, *husb, *wife, *chil, *body;
    splitFamily(fam, &refns, &husb, &wife, &chil, &body);
    joinFamily(fam, refns, husb, wife, chil, body);
    return fam;
}

// normalizeEvent -- Convert an event record into standard format.
//--------------------------------------------------------------------------------------------------
GNode *normalizeEvent(GNode *event)
{
  GNode *refns, *body;
  splitEvent (event, &refns, &body);
  joinEvent (event, refns, body);

  return event;
}

// normalizeSource -- Convert a source record into standard format.
//--------------------------------------------------------------------------------------------------
GNode *normalizeSource(GNode *source)
{
  GNode *refns, *body;
  splitSource (source, &refns, &body);
  joinSource (source, refns, body);

  return source;
}

// normalizeOther -- Convert an other record into standard format.
//--------------------------------------------------------------------------------------------------
GNode *normalizeOther(GNode *other)
{
  GNode *refns, *body;
  splitOther (other, &refns, &body);
  joinOther (other, refns, body);

  return other;
}

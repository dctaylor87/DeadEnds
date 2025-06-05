/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
/*=============================================================
 * remove.c -- Remove child or spouse from family
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 05 Dec 94
 *   3.0.3 - 15 Aug 95
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "standard.h"
#include "denls.h"

#include "hashtable.h"
#include "refnindex.h"
#include "database.h"
#include "gedcom.h"
#include "gnode.h"
#include "sequence.h"
#include "errors.h"
#include "feedback.h"
#include "remove.h"
#include "splitjoin.h"
#include "messages.h"
#include "ll-addoperations.h"
#include "rfmt.h"
#include "refns.h"
#include "browse.h"
#include "name.h"

/*********************************************
 * local function prototypes
 *********************************************/

static GNode *remove_any_xrefs_node_list(CString xref, GNode *list);
static void remove_refn_list(GNode *refn, CString key, Database *database);

/*================================================================
 * remove_indi_by_root -- Delete person and links; if this leaves families
 *   with no links, remove them.
 *   This should not fail.
 *  indi:  [in]  person to remove - (must be valid)
 * Created: 2001/11/08, Perry Rapp
 *==============================================================*/
void
remove_indi_by_root (GNode *indi, Database *database)
{
	String key = nxref(indi);
	GNode *name, *refn, *sex, *body, *famc, *fams;
	GNode *node, *husb, *wife, *chil, *rest, *fam, *fref;

/* Factor out portions critical to lifelines (lineage-linking, names, & refns) */
	splitPerson(indi, &name, &refn, &sex, &body, &famc, &fams);

/* Remove person from families he/she is in as a parent */

	for (node = fams; node; node = nsibling(node)) {
		fam = keyToFamily(nval(node), database->recordIndex);
		splitFamily(fam, &fref, &husb, &wife, &chil, &rest);
		/* remove all occurrences of this person as spouse */
		husb = remove_any_xrefs_node_list(nxref(indi), husb);
		wife = remove_any_xrefs_node_list(nxref(indi), wife);
		joinFamily(fam, fref, husb, wife, chil, rest);
		if (! husb && ! wife && ! chil)
			remove_empty_fam (fam, database);
	}

/* Remove person from families he/she is in as a child */

	for (node = famc; node; node = nsibling(node)) { 
		fam = keyToFamily(nval(node), database->recordIndex);
		splitFamily(fam, &fref, &husb, &wife, &chil, &rest);
		/* remove all occurrences of this person as child */
		chil = remove_any_xrefs_node_list(nxref(indi), chil);
		joinFamily(fam, fref, husb, wife, chil, rest);
		if (! husb && ! wife && ! chil)
			remove_empty_fam (fam, database);
	}


/* Remove any name and refn entries */
	removeNamesOfPersonFromIndex (database->nameIndex, indi);
	remove_refn_list(refn, key, database);

/* Reassemble & delete the in-memory record we're holding (indi) */
	joinPerson(indi, name, refn, sex, body, famc, fams);

/* Remove any entries in existing browse lists */
	remove_from_browse_lists(key);
}
/*==========================================
 * remove_empty_fam -- Delete family from database
 *  This will call message & fail if there are any
 *  people in the family.
 *========================================*/
bool
remove_empty_fam (GNode *fam, Database *database)
{
	String key;
	GNode *husb, *wife, *chil, *rest, *refn;

	if (!fam) return true;
	key = nxref(fam);

/* Factor out portions critical to lifelines (lineage-linking, names, & refns) */
	splitFamily(fam, &refn, &husb, &wife, &chil, &rest);

	/* We're only supposed to be called for empty families */
	if (husb || wife || chil) {
		/* TO DO - This probably should never happen, and maybe could be
		changed to an assertion, 2001/11/08, Perry, but I've not checked
		merge code's call */
		msg_error("%s", _(qShaslnk));
		joinFamily(fam, refn, husb, wife, chil, rest);
		return false;
	}

	joinFamily(fam, refn, husb, wife, chil, rest);

	/* Remove any refn entries */
	remove_refn_list(refn, key, database);

	return true;
}
/*=========================================
 * remove_child -- Remove child from family
 *  silent function
 *=======================================*/
bool
remove_child (GNode *indi, GNode *fam, Database *database)
{
	GNode *node, *last;

/* Make sure child is in family and remove his/her CHIL line */
	if (!(node = findNode(fam, "CHIL", nxref(indi), &last))) {
		return false;
	}
	if (last)
		nsibling(last) = nsibling(node);
	else
		nchild(fam) = nsibling(node);
	freeGNode(node);

/* Remove FAMC line from child */
	node = findNode(indi, "FAMC", nxref(fam), &last);
	if (last)
		nsibling(last) = nsibling(node);
	else
		nchild(indi) = nsibling(node);
	freeGNode(node);

	/* Update database with changed records */
	if (num_fam_xrefs(fam) == 0)
		remove_empty_fam(fam, database);
	return true;
}
/*===========================================
 * remove_spouse -- Remove spouse from family
 *  both arguments required
 *  silent function
 *=========================================*/
bool
remove_spouse (GNode *indi, GNode *fam, Database *database)
{
	GNode *node=0, *last=0;

/* Remove (one) reference from family */
	node = findNode(fam, "HUSB", nxref(indi), &last);
	if (!node) {
		node = findNode(fam, "WIFE", nxref(indi), &last);
	}
	if (!node)
		return false;

	if (last)
		nsibling(last) = nsibling(node);
	else
		nchild(fam) = nsibling(node);
	freeGNode(node);
	node = NULL;

/* Remove (one) FAMS line from spouse */
	node = findNode(indi, "FAMS", nxref(fam), &last);
	ASSERT(node);
	ASSERT(last);
	nsibling(last) = nsibling(node);
	freeGNode(node);
	node = NULL;

	if (num_fam_xrefs(fam) == 0)
		remove_empty_fam(fam, database);
	return true;
}
/*================================================================
 * remove_fam_record -- Delete family (and any family lineage linking)
 *   This should not fail.
 *  fam:  [in]  family to remove - (must be valid)
 * Created: 2005/01/08, Perry Rapp
 *==============================================================*/
bool
remove_fam_record (ATTRIBUTE_UNUSED GNode *frec)
{
	msg_error("%s", _("Families may not yet be removed in this fashion."));
	return false;
}

/*================================================================
 * remove_any_record -- Delete record (and any family lineage linking)
 *   This should not fail.
 *  record:  [in]  record to remove - (must be valid)
 * Created: 2005/01/08, Perry Rapp
 *==============================================================*/
bool
remove_any_record (GNode *record, Database *database)
{
	GNode *root=0;
	ASSERT(record && ! record->parent);
	CString key = nzkey(record);
	GNode *rest, *refn;

	/* indi & family records take special handling, for lineage-linking */
	if (nztype(record) == GRPerson) {
		remove_indi_by_root(nztop(record), database);
		return true;
	}
	if (nztype(record) == GRFamily) {
		return remove_fam_record(record);
	}

	root = nztop(record);
	
/* Factor out portions critical to lifelines (refns) */
	splitOther(root, &refn, &rest);

	record=NULL; /* record no longer valid */

/* Remove any refn entries */
	remove_refn_list(refn, key, database);

#if !defined(DEADENDS)
/* Remove from on-disk database */
	del_in_dbase(key);
#endif

/* Reassemble & delete the in-memory record we're holding (root) */
	joinOther(root, refn, rest);
	freeGNodes(root);

	return true;
}
/*=======================================================
 * num_fam_xrefs -- Find number of person links in family
 *   LOOSEEND -- How about other links in the future???
 *=====================================================*/
int
num_fam_xrefs (GNode *fam)
{
	int num;
	GNode *fref, *husb, *wife, *chil, *rest;

	splitFamily(fam, &fref, &husb, &wife, &chil, &rest);
	num = lengthGNodes(husb) + lengthGNodes(wife) + lengthGNodes(chil);
	joinFamily(fam, fref, husb, wife, chil, rest);
	return num;
}
/*==========================================
 * remove_any_xrefs_node_list -- 
 *  Remove from list any occurrences of xref
 * Eg, removing @I2@ from a husband list of a family
 * Returns head of list (which might be different if first node removed)
 *========================================*/
static GNode *
remove_any_xrefs_node_list (CString xref, GNode *list)
{
	GNode *prev = NULL;
	GNode *curr = list;
	GNode *rtn = list;

	ASSERT(xref);

	while (curr) {
		GNode *next = nsibling(curr);
		if (eqstr(xref, nval(curr))) {
			if (prev)
				nsibling(prev) = next;
			else
				rtn = next;
			nsibling(curr) = NULL;
			freeGNodes(curr);
		}
		prev = curr;
		curr = next;
	}
	return rtn;
}

#if !defined(DEADENDS)
/*=========================================
 * remove_name_list -- Remove all names in passed list
 *  key is key of record to which name chain belongs
 *  silent function, does not fail
 *=======================================*/
static void
remove_name_list (GNode *name, CString key)
{
	GNode *node=0;
	for (node = name; node; node = nsibling(node)) {
		remove_name(nval(node), key);
	}
}
#endif

/*=========================================
 * remove_refn_list -- Remove all refns in passed list
 *  key is key of record to which refn chain belongs
 *  silent function, does not fail
 *=======================================*/
static void
remove_refn_list (GNode *refn, CString key, Database *database)
{
	GNode *node;
	for (node = refn; node; node = nsibling(node)) {
		if (nval(node)) 
			removeRefn(nval(node), key, database);
	}
}

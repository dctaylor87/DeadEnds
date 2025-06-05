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
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/*=============================================================
 * replace.c -- Replace persons and families
 * Copyright(c) 1995-96 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.3 - 20 Jan 96
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "standard.h"
#include "denls.h"

#include "list.h"
#include "zstr.h"
#include "hashtable.h"
#include "refnindex.h"
#include "database.h"
#include "gnode.h"
#include "nameindex.h"
#include "rfmt.h"
#include "translat.h"
#include "xlat.h"
#include "editing.h"
#include "splitjoin.h"
#include "nodeutils.h"
#include "replace.h"
#include "refns.h"
#include "ll-addoperations.h"
#include "sequence.h"
#include "browse.h"
#include "name.h"

/*===================================================================
 * replace_indi -- Replace a person in database with modified version
 *  indi1 = current record (copy from database)
 *  indi2 = new data
 *  replaces all children nodes of indi1 with children nodes of indi2
 *  consumes indi2 (calls free_node on it)
 *=================================================================*/
void
replace_indi (GNode *indi1, GNode *indi2, Database *database)
{
	GNode *name1, *name2, *refn1, *refn2, *sex, *body, *famc, *fams;
	GNode *node, *namen, *refnn, *name1n, *refn1n, *indi0;
	String key;


	/* Move indi1 data into indi0 & delete it (saving names & refns */
	splitPerson(indi1, &name1, &refn1, &sex, &body, &famc, &fams);
	indi0 = copyGNode(indi1);
	joinPerson(indi0, NULL, NULL, sex, body, famc, fams);
	freeGNodes(indi0);
	/* Move indi2 data into indi1, also copy out lists of names & refns */
	splitPerson(indi2, &name2, &refn2, &sex, &body, &famc, &fams);
	namen = copyGNodes(name2, true, true);
	refnn = copyGNodes(refn2, true, true);
	joinPerson(indi1, name2, refn2, sex, body, famc, fams);
	freeGNode(indi2);
#if !defined(DEADENDS)
	nodechk(indi1, "replace_indi");
#endif

	/* Write data to database */

	key = nxref(indi1);
	/* update name & refn info */
	/* classify does a diff on its first two arguments, repopulating all three
	arguments -- first is left-only, second is right-only, third is shared */
	/* Note: classify eliminates duplicates */
	classifyNodes(&name1, &namen, &name1n);
	classifyNodes(&refn1, &refnn, &refn1n);
	for (node = name1; node; node = nsibling(node))
		removeFromNameIndex(database->nameIndex, nval(node), key);
	for (node = namen; node; node = nsibling(node))
		insertInNameIndex(database->nameIndex, nval(node), key);
	rename_from_browse_lists(key);
	for (node = refn1; node; node = nsibling(node))
		if (nval(node)) removeRefn(nval(node), key, database);
	for (node = refnn; node; node = nsibling(node))
		if (nval(node)) addRefn(nval(node), key, database);

/* now cleanup (indi1 tree is now composed of indi2 data) */
	freeGNodes(name1);
	freeGNodes(namen);
	freeGNodes(name1n);
	freeGNodes(refn1);
	freeGNodes(refnn);
	freeGNodes(refn1n);
}
/*==================================================================
 * replace_fam -- Replace a family in database with modified version
 *  fam1 = current record (copy from database)
 *  fam2 = new data
 *  replaces all children nodes of fam1 with children nodes of fam2
 *  consumes fam2 (calls free_node on it)
 *================================================================*/
void
replace_fam (GNode *fam1, GNode *fam2, Database *database)
{
	GNode *refn1, *refn2, *husb, *wife, *chil, *body;
	GNode *refnn, *refn1n, *node, *fam0;
	String key;


	/* Move fam1 data into fam0 & delete it (saving refns) */
	splitFamily(fam1, &refn1, &husb, &wife, &chil, &body);
	fam0 = copyGNode(fam1);
	joinFamily(fam0, NULL, husb, wife, chil, body);
	freeGNodes(fam0);
	/* Move fam2 data into fam1, also copy out list of refns */
	splitFamily(fam2, &refn2, &husb, &wife, &chil, &body);
	refnn = copyGNodes(refn2, true, true);
	joinFamily(fam1, refn2, husb, wife, chil, body);
	freeGNode(fam2);

	/* Write data to database */
	
#if !defined(DEADENDS)
	fam_to_dbase(fam1);
#endif
	key = nxref(fam1);
	/* remove deleted refns & add new ones */
	classifyNodes(&refn1, &refnn, &refn1n);
	for (node = refn1; node; node = nsibling(node))
		if (nval(node)) removeRefn(nval(node), key, database);
	for (node = refnn; node; node = nsibling(node))
		if (nval(node)) addRefn(nval(node), key, database);
	freeGNodes(refn1);
	freeGNodes(refnn);
	freeGNodes(refn1n);
}

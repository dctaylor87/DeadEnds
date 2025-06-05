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
 * advedit.c -- Advanced edit features
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.2 - 11 Dec 94
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "denls.h"

#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "hashtable.h"
#include "refnindex.h"
#include "database.h"
#include "gedcom.h"
#include "gnode.h"
#include "xlat.h"
#include "readwrite.h"
#include "rfmt.h"
#include "sequence.h"
#include "ask.h"
#include "llinesi.h"
#include "errors.h"
#include "feedback.h"
#include "liflines.h"
#include "pvalue.h"
#include "editing.h"
#include "ll-node.h"
#include "xref.h"

#include "llpy-externs.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static GNode *expand_tree(GNode *);
static bool advedit_expand_traverse(GNode *, void *param);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=================================================================
 * expand_tree -- Create copy of node tree with additional link info
 *===============================================================*/
static GNode *
expand_tree (GNode *root0)
{
	GNode *copy, *node, *sub;
	String key;
	static GNode *root;	/* root of record being edited */
	List *subs;	/* list of contained records */
	GNode *expd;	/* expanded main record - copy - our retval */

	root = root0;
	expd = copyGNodes(root, true, true);
	subs = createList (NULL, NULL, NULL, false);
	traverse_nodes(expd, advedit_expand_traverse, subs);

   /* expand the list of records into the copied record */
	FORLIST(subs, el)
		node = (GNode *) el;
#ifdef DEBUG
		llwprintf("in list: %s %s\n", ntag(node), nval(node));
#endif
		key = nval(node);
		if ((sub = nztop (getRecord (key, database->recordIndex)))) {
			copy = copy_node_subtree(sub);
			nxref(node)    = nxref(copy);
			ntag(node)     = ntag(copy);
			nchild(node)   = nchild(copy);
			nparent(node)  = nparent(copy);
/*MEMORY LEAK; MEMORY LEAK; MEMORY LEAK: node not removed (because its
  value and possibly xref [probably not] are still being referred to */
		}
	ENDLIST
	/* Shouldn't we free subs now ? Perry 2001/06/22 */
#ifdef DEBUG
	showGNode (expd);
#endif
	return expd;
}

/*=================================================================
 * advanced_person_edit --
 *===============================================================*/
void
advanced_person_edit (GNode *root0)
{
	FILE *fp;
	GNode *expd;

#ifdef DEBUG
	llwprintf("advanced_person_edit: %s %s %s\n", nxref(root0), 
		  ntag(root0),nval(root0));
#endif
	expd = expand_tree(root0);
	ASSERT(fp = fopen(editfile, DEWRITETEXT));
	write_nodes(0, fp, NULL, expd, true, true, true);
	fclose(fp);
	do_edit();
}

/*=================================================================
 * advanced_family_edit --
 *===============================================================*/
void
advanced_family_edit (GNode *root0)
{
	FILE *fp;
	GNode *expd;

#ifdef DEBUG
	llwprintf("advanced_family_edit: %s %s %s\n", nxref(root0),
		  ntag(root0),nval(root0));
#endif
	expd = expand_tree(root0);
	ASSERT(fp = fopen(editfile, DEWRITETEXT));
	write_nodes(0, fp, NULL, expd, true, true, true);
	fclose(fp);
	do_edit();
}
/*=================================================================
 * advedit_expand_traverse -- Traverse routine called when expanding record
 *===============================================================*/
static bool
advedit_expand_traverse (GNode *node, void *param)
{
	List *subs = (List *)param;
	CString key = value_to_xref(nval(node));
	if (!key) return true;
	key = strsave(key);
#ifdef DEBUG
	llwprintf("expand_traverse: %s %s\n", ntag(node), nval(node));
#endif /* DEBUG */
	FORLIST(subs, el)
#ifdef DEBUG
	llwprintf("expand_traverse: %s %s\n", key, nval((GNode *) el));
#endif /* DEBUG */
		if (eqstr(key, nval((GNode *) el))) {
			stdfree(key);
			return true;
		}
	ENDLIST
	enqueueList(subs, node);
	stdfree(key);
	return true;
}

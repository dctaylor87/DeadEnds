/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */

/* This code originated as src/gedlib/node.c in Lifelines.  Parts not
   needed for DeadEnds were removed.  The remainder was modified for
   DeadEnds by David Taylor */
/*=============================================================
 * node.c -- Standard GEDCOM NODE operations
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 04 Sep 93
 *   3.0.0 - 29 Aug 94    3.0.2 - 23 Dec 94
 *   3.0.3 - 16 Jan 96
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "porting.h"
#include "standard.h"
#include "llnls.h"

#include "zstr.h"
#include "list.h"
#include "refnindex.h"
#include "gnode.h"
#include "recordindex.h"
#include "hashtable.h"
#include "stringtable.h"
#include "database.h"
#include "rfmt.h"
#include "readwrite.h"
#include "codesets.h"
#include "translat.h"
#include "feedback.h"
#include "messages.h"
#include "gedcom.h"
#include "lineage.h"
#include "llpy-externs.h"	/* XXX need to move __llpy_key_to_record elsewhere XXX */
#include "ll-node.h"
#include "de-strings.h"

/*********************************************
 * global/exported variables
 *********************************************/


/*********************************************
 * local types
 *********************************************/

/* node allocator's freelist */
typedef struct blck *NDALLOC;
struct blck { NDALLOC next; };

/*********************************************
 * local enums & defines
 *********************************************/

enum { NEW_RECORD, EXISTING_LACKING_WH_RECORD };

/*********************************************
 * local function prototypes, alphabetical
 *********************************************/

//#define alloc_node(msg) alloc_node_int(msg,__FILE__,__LINE__)
//static GNode *alloc_node_int(char* msg, char *file, int line);
//static String fixtag (String tag);
static RecordIndexEl *indi_to_prev_sib_impl(GNode *indi, Database *database);
static int node_strlen(int levl, GNode *node);

/*********************************************
 * local variables
 *********************************************/

/* node allocator's free list */
List *alloc_block_list = (List *) 0;
//static NDALLOC first_blck = (NDALLOC) 0;
//static int live_count = 0;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

#if !defined(DEADENDS)		/* single caller if #if'ed out */
/*=============================
 * fixtag -- Keep tags in table
 * returns pointer to table's memory
 *===========================*/
static String
fixtag (String tag)
{
	String str = searchStringTable(tagtable, tag);
	if (!str) {
		addToStringTable(tagtable, tag, tag);
		str = searchStringTable(tagtable, tag);
	}
	return str;
}
#endif

#if !defined(DEADENDS)	       /* seems useful, but no current callers */
/*=====================================
 * change_node_tag -- Give new tag to node
 *===================================*/
void
change_node_tag (GNode *node, String newtag)
{
	/* tag belongs to tagtable, so don't free old one */
	ntag(node) = fixtag(newtag);
}
#endif

#if 0			  /* DeadEnds has freeGNodes */
/* free_nodes -- Free all NODEs in tree.  */

void
free_nodes (GNode *node)
{
	GNode *sib;
	while (node) {
		if (nchild(node)) {
			free_nodes(nchild(node));
			nchild (node) = 0;
		}
		sib = nsibling(node);
		free_node(node,"free_nodes");
		node = sib;
	}
}
#endif

/*==============================================================
 * tree_strlen -- Compute string length of tree -- don't count 0
 *============================================================*/
int
tree_strlen (int levl,       /* level */
             GNode *node)      /* root */
{
	int len = 0;
	while (node) {
		len += node_strlen(levl, node);
		if (nchild(node))
			len += tree_strlen(levl + 1, nchild(node));
		node = nsibling(node);
	}
	return len;
}
/*================================================================
 * node_strlen -- Compute NODE string length -- count \n but not 0
 *==============================================================*/
static int
node_strlen (int levl,       /* level */
             GNode *node)      /* node */
{
	int len;
	char scratch[10];
	snprintf(scratch, sizeof(scratch), FMT_INT, levl);
	len = strlen(scratch) + 1;
	if (nxref(node)) len += strlen(nxref(node)) + 1;
	len += strlen(ntag(node));
	if (nval(node)) len += strlen(nval(node)) + 1;
	return len + 1;
}

#if !defined(DEADENDS)
/*==========================================
 * unknown_node_to_dbase -- Store node of unknown type
 *  in database
 * Created: 2001/04/06, Perry Rapp
 *========================================*/
void
unknown_node_to_dbase (GNode *node)
{
	/* skip tag validation */
	node_to_dbase(node, NULL);
}
#endif

/*===============================================
 * next_spouse -- Return next spouse of family
 * GNode **node     [in/out] pass in nchild(fam) to start
 *                or nsibling(previous node returned from this routine) to continue
 * RECORD *spouse [out]     next spouse in family
 * returns 1 for success, -1 if next HUSB/WIFE record is invalid
 *         0 no more spouses found
 * returns addref'd record in *spouse
 *=============================================*/
int
next_spouse (GNode **node, RecordIndexEl **spouse, Database *database)
{
	CString key=0;
	if (!node || !spouse) return 0;
	while (*node) {
	    if (eqstr(ntag(*node),"HUSB") || eqstr(ntag(*node),"WIFE")) {
		key = rmvat(nval(*node));
		if (!key) return -1;
		*spouse = keyToPersonRecord(key, database);
		if (!*spouse) return -1;
		return 1;
	    }
	    *node = nsibling(*node);
	}
	return 0;
}

/*==================================================
 * indi_to_prev_sib -- Return previous sib of person
 *  returns addref'd record
 *================================================*/
static RecordIndexEl *
indi_to_prev_sib_impl (GNode *indi, Database *database)
{
	GNode *fam, *prev, *node;
	if (!indi) return NULL;
	if (!(fam = personToFamilyAsChild(indi, database))) return NULL;
	prev = NULL;
	node = CHIL(fam);
	/* loop thru all nodes following first child, keeping most recent CHIL */
	while (node) {
		if (eqstr(nxref(indi), nval(node))) {
			if (!prev) return NULL;
			return keyToPersonRecord(rmvat(nval(prev)), database);
		}
		if (eqstr(ntag(node),"CHIL"))
			prev = node;
		node = nsibling(node);
	}
	return NULL;
}
RecordIndexEl *
indi_to_prev_sib (RecordIndexEl *irec, Database *database)
{
	return indi_to_prev_sib_impl(nztop(irec), database);
}
/*==============================================
 * indi_to_next_sib -- Return next sib of person
 *  returns addref'd record
 *============================================*/
static RecordIndexEl *
indi_to_next_sib_impl (GNode *indi, Database *database)
{
	GNode *fam, *node;
	bool found;
	if (!indi) return NULL;
	if (!(fam = personToFamilyAsChild(indi, database))) return NULL;
	node = CHIL(fam);
	found = false;  /* until we find indi */
	while (node) {
		if (!found) {
			if (eqstr(nxref(indi), nval(node)))
				found = true;
		} else {
			if (eqstr(ntag(node),"CHIL"))
				return keyToPersonRecord(rmvat(nval(node)), database);
		}
		node = nsibling(node);
	}
	return NULL;
}

RecordIndexEl *
indi_to_next_sib (RecordIndexEl *irec, Database *database)
{
	return indi_to_next_sib_impl(nztop(irec), database);
}

/*======================================
 * node_to_tag -- Return a subtag of a node
 * (presumably top level, but not necessarily)
 *====================================*/
String node_to_tag (GNode *node, String tag, int len)
{
	/* GEDCOM has no max, but MAXLINELEN is the longest line we support. */
	static char scratch[MAXLINELEN+1];
	String refn;
	if (!node) return NULL;
	if (!(node = findTag(nchild(node), tag)))
		return NULL;
	refn = nval(node);
	if (len > (int)sizeof(scratch)-1)
		len = sizeof(scratch)-1;
	destrsets(scratch, len, uu8, refn);
	return scratch;
}
/*==============================================
 * record_to_date_place -- Find event info
 *  record:  [IN]  record to search
 *  tag:     [IN]  desired tag (eg, "BIRT")
 *  date:    [OUT] date found (optional)
 *  plac:    [OUT] place found (option)
 *  count:   [I/O] #instances of tag found (caller must init)
 * Created: 2003-01-12 (Perry Rapp)
 *============================================*/
void
record_to_date_place (RecordIndexEl *record, String tag, String * date, String * plac
	, int * count)
{
	GNode *node=0;
	for (node = record_to_first_event(record, tag)
		; node
		; node = node_to_next_event(node, tag)) {
		if (++(*count) == 1)
			/* only record first instance */
			event_to_date_place(node, date, plac);
	}
}
/*==============================================
 * record_to_first_event -- Find requested event subtree
 *  record:  [IN]  record to search
 *  tag:     [IN]  desired tag (eg, "BIRT")
 * Created: 2003-01-12 (Perry Rapp)
 *============================================*/
GNode *
record_to_first_event (RecordIndexEl *record, CString tag)
{
	GNode *node = nztop(record);
	if (!node) return NULL;
	return findTag(nchild(node), tag);
}
/*==============================================
 * node_to_next_event -- Find next event after node
 *  node:  [IN]  start search after node
 *  tag:   [IN]  desired tag (eg, "BIRT")
 * Created: 2003-01-12 (Perry Rapp)
 *============================================*/
GNode *
node_to_next_event (GNode *node, CString tag)
{
	return findTag(nsibling(node), tag);
}

/*===========================================
 * event_to_date_place  -- Find date & place
 *  node:  [IN]  node tree of event to describe
 *  date:  [OUT] value of first DATE line (optional)
 *  plac:  [OUT] value of first PLACE line (optional)
 *=========================================*/
void
event_to_date_place (GNode *node, String * date, String * plac)
{
	int count=0;
	if (date) {
		*date = NULL;
	} else {
		++count;
	}
	if (plac) {
		*plac = NULL;
	} else {
		++count;
	}
	if (!node) return;
	node = nchild(node);
	while (node && count<2) {
		if (eqstr("DATE", ntag(node)) && date && !*date) {
			*date = nval(node);
			++count;
		}
		if (eqstr("PLAC", ntag(node)) && plac && !*plac) {
			*plac = nval(node);
			++count;
		}
		node = nsibling(node);
	}
}

#if 0			 /* DeadEnds now has eventToString */
/*===========================================
 * event_to_string -- Convert event to string
 * Finds DATE & PLACE nodes, and prints a string
 * representation of them.
 *  node:  [IN]  node tree of event to describe
 *  ttm:   [IN]  translation table to use
 *  rfmt:  [IN]  reformatting info (may be NULL)
 *=========================================*/
String
event_to_string (GNode *node, RFMT rfmt)
{
	static char scratch1[MAXLINELEN+1];
	String date, plac;
	event_to_date_place(node, &date, &plac);
	if (!date && !plac) return NULL;
	/* Apply optional, caller-specified date & place reformatting */
	if (rfmt && date && rfmt->rfmt_date)
		date = (*rfmt->rfmt_date)(date);
	if (rfmt && plac && rfmt->rfmt_plac)
		plac = (*rfmt->rfmt_plac)(plac);
	if (rfmt && rfmt->combopic && date && date[0] && plac && plac[0]) {
		sprintpic2(scratch1, sizeof(scratch1), uu8, rfmt->combopic, date, plac);
	} else if (date && date[0]) {
		destrncpy(scratch1, date, sizeof(scratch1), uu8);
	} else if (plac && plac[0]) {
		destrncpy(scratch1, plac, sizeof(scratch1), uu8);
	} else {
		return NULL;
	}
	return scratch1;
}
#endif

#if 0				/* DeadEnds has showGNode, showGNodeRecursive */
/*================================
 * show_node -- Show tree -- DEBUG
 *==============================*/
void
show_node (GNode *node)
{
	if (!node) llwprintf("%s", "(NIL)");
	show_node_rec(0, node);
}
/*================================================
 * show_node_rec -- Recursive version of show_node
 *==============================================*/
void
show_node_rec (int levl,
               GNode *node)
{
	int i;
	if (!node) return;
	for (i = 1;  i < levl;  i++)
		llwprintf("%s", "  ");
	llwprintf(FMT_INT, levl);
	if (nxref(node)) llwprintf(" %s", nxref(node));
	llwprintf(" %s", ntag(node));
	if (nval(node)) llwprintf(" %s", nval(node));
	llwprintf("%s", "\n");
	show_node_rec(levl + 1, nchild(node));
	show_node_rec(levl    , nsibling(node));
}
#endif

/*========================
 * copy_node_subtree -- Copy tree
 *======================*/
GNode *
copy_node_subtree (GNode *node)
{
	return copyNodes(node, true, false);
}

/*===============================================================
 * traverse_nodes -- Traverse nodes in tree while doing something
 * GNode *node:  root of tree to traverse
 * func:         function to call at each node (returns false to stop traversal)
 * param:        opaque pointer for client use, passed thru to callback
 *=============================================================*/
bool
traverse_nodes (GNode *node, bool (*func)(GNode *, Word), Word param)
{
	while (node) {
		if (!(*func)(node, param)) return false;
		if (nchild(node)) {
			if (!traverse_nodes(nchild(node), func, param))
				return false;
		}
		node = nsibling(node);
	}
	return true;
}

#if 0		  /* no callers */
/*==================================================
 * num_spouses_of_indi -- Returns number of spouses of person
 *================================================*/
int
num_spouses_of_indi (GNode *indi)
{
	int nsp;
	if (!indi) return 0;
#if defined(DEADENDS)
	FORSPOUSES(indi, spouse, fam, nsp, database) ENDSPOUSES
#else
	FORSPOUSES(indi, spouse, fam, nsp) ENDSPOUSES
#endif
	return nsp;  /* don't include self*/
}
#endif

#if !defined(DEADENDS)
/*=================================================
 * check_node_leaks -- Called when database closing
 *  for debugging
 *===============================================*/
void
check_node_leaks (void)
{
	if (live_count) {
		if (fpleaks) {
			LLDATE date;
			get_current_lldate(&date);
			fprintf(fpleaks, _("Node memory leaks:"));
			fprintf(fpleaks, " %s", date.datestr);
			fprintf(fpleaks, "\n  ");
			fprintf(fpleaks, _pl("%d item leaked", "%d items leaked", live_count), live_count);
			fprintf(fpleaks, "\n");
		}
	}
}
#endif

/*=================================================
 * term_node_allocator -- Called to free all nodes in the free list
 *===============================================*/
void
term_node_allocator (void)
{
	deleteList(alloc_block_list);
}

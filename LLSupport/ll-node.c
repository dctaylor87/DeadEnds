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

#if defined(DEADENDS)

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
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
//#include "gedcomi.h"
#include "feedback.h"
//#include "metadata.h"
//#include "date.h"
#include "messages.h"
//#include "leaksi.h"
#include "gedcom.h"
#include "lineage.h"
#include "ll-node.h"
#include "de-strings.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase
#else

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "gedcomi.h"
#include "feedback.h"
#include "metadata.h"
#include "lloptions.h"
#include "date.h"
#include "messages.h"
#include "leaksi.h"

#endif

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
//static NODE alloc_node_int(char* msg, char *file, int line);
//static STRING fixtag (STRING tag);
static RECORD indi_to_prev_sib_impl(NODE indi);
static INT node_strlen(INT levl, NODE node);

/*********************************************
 * local variables
 *********************************************/

/* node allocator's free list */
LIST alloc_block_list = (LIST) 0;
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
static STRING
fixtag (STRING tag)
{
	STRING str = valueof_str(tagtable, tag);
	if (!str) {
		insert_table_str(tagtable, tag, tag);
		str = valueof_str(tagtable, tag);
	}
	return str;
}
#endif

#if !defined(DEADENDS)	       /* seems useful, but no current callers */
/*=====================================
 * change_node_tag -- Give new tag to node
 *===================================*/
void
change_node_tag (NODE node, STRING newtag)
{
	/* tag belongs to tagtable, so don't free old one */
	ntag(node) = fixtag(newtag);
}
#endif

/*===========================
 * create_temp_node -- Create NODE for temporary use
 *  (not to be connected to a record)
 * [All arguments are duplicated, so caller doesn't have to]
 * STRING xref  [in] xref
 * STRING tag   [in] tag
 * STRING val:  [in] value
 * NODE prnt:   [in] parent
 * Created: 2003-02-01 (Perry Rapp)
 *=========================*/
NODE
create_temp_node (STRING xref, STRING tag, STRING val, NODE prnt)
{
	NODE node = create_node(xref, tag, val, prnt);
	nflag(node) = ND_TEMP;
	return node;
}
/*===========================
 * free_temp_node_tree -- Free a node created by create_temp_node
 * If the reference count is non-zero, we do not delete it nor its
 * children -- siblings are still fair game, though.
 * Created: 2003-02-01 (Perry Rapp).  Modified: David Taylor.
 *=========================*/
void
free_temp_node_tree (NODE node)
{
	NODE n2;
	if (get_nrefcnt (node) == 0) {
		if ((n2 = nchild(node))) {
			free_temp_node_tree(n2);
			nchild(node) = 0;
		}
	}
	if ((n2 = nsibling(node))) {
		free_temp_node_tree(n2);
		nsibling(node) = 0;
	}
	if (get_nrefcnt (node) == 0) {
		free_node(node,"free_temp_node_tree");
	}
}
/*===================================
 * is_temp_node -- Return whether node is a temp
 * Created: 2003-02-04 (Perry Rapp)
 *=================================*/
BOOLEAN
is_temp_node (NODE node)
{
	return !!(nflag(node) & ND_TEMP);
}

/* set_temp_node_helper -- {sets|clears} ND_TEMP in node, children,
   and siblings */

static void
set_temp_node_helper (NODE node, BOOLEAN temp)
{
  if (is_temp_node (node) ^ temp)
    nflag (node) ^= ND_TEMP;
  if (nchild (node))
    set_temp_node_helper (nchild (node), temp);
  if (nsibling (node))
    set_temp_node_helper (nsibling (node), temp);
}

/*===================================
 * set_temp_node -- make node temp (or not)
 * Created: 2003-02-04 (Perry Rapp)
 *=================================*/
void
set_temp_node (NODE node, BOOLEAN temp)
{
	if (is_temp_node(node) ^ temp)
		nflag(node) ^= ND_TEMP;
	if (nchild (node))
	  set_temp_node_helper (nchild (node), temp);
}
/*==============================================================
 * tree_strlen -- Compute string length of tree -- don't count 0
 *============================================================*/
INT
tree_strlen (INT levl,       /* level */
             NODE node)      /* root */
{
	INT len = 0;
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
static INT
node_strlen (INT levl,       /* level */
             NODE node)      /* node */
{
	INT len;
	char scratch[10];
	snprintf(scratch, sizeof(scratch), FMT_INT, levl);
	len = strlen(scratch) + 1;
	if (nxref(node)) len += strlen(nxref(node)) + 1;
	len += strlen(ntag(node));
	if (nval(node)) len += strlen(nval(node)) + 1;
	return len + 1;
}
/*==========================================
 * unknown_node_to_dbase -- Store node of unknown type
 *  in database
 * Created: 2001/04/06, Perry Rapp
 *========================================*/
void
unknown_node_to_dbase (NODE node)
{
	/* skip tag validation */
	node_to_dbase(node, NULL);
}

/*===============================================
 * next_spouse -- Return next spouse of family
 * NODE *node     [in/out] pass in nchild(fam) to start
 *                or nsibling(previous node returned from this routine) to continue
 * RECORD *spouse [out]     next spouse in family
 * returns 1 for success, -1 if next HUSB/WIFE record is invalid
 *         0 no more spouses found
 * returns addref'd record in *spouse
 *=============================================*/
int
next_spouse (NODE *node, RECORD *spouse)
{
	CNSTRING key=0;
	if (!node || !spouse) return 0;
	while (*node) {
	    if (eqstr(ntag(*node),"HUSB") || eqstr(ntag(*node),"WIFE")) {
		key = rmvat(nval(*node));
		if (!key) return -1;
		*spouse = qkey_to_irecord(key);
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
static RECORD
indi_to_prev_sib_impl (NODE indi)
{
#if defined(DEADENDS)
	GNode *fam, *prev, *node;
#else
	NODE fam, prev, node;
#endif
	if (!indi) return NULL;
	if (!(fam = indi_to_famc(indi))) return NULL;
	prev = NULL;
	node = CHIL(fam);
	/* loop thru all nodes following first child, keeping most recent CHIL */
	while (node) {
		if (eqstr(nxref(indi), nval(node))) {
			if (!prev) return NULL;
			return key_to_record(rmvat(nval(prev)));
		}
		if (eqstr(ntag(node),"CHIL"))
			prev = node;
		node = nsibling(node);
	}
	return NULL;
}
RECORD
indi_to_prev_sib (RECORD irec)
{
	return indi_to_prev_sib_impl(nztop(irec));
}
/*==============================================
 * indi_to_next_sib -- Return next sib of person
 *  returns addref'd record
 *============================================*/
static RECORD
indi_to_next_sib_impl (NODE indi)
{
#if defined(DEADENDS)
	GNode *fam, *node;
#else
	NODE fam, node;
#endif
	BOOLEAN found;
	if (!indi) return NULL;
	if (!(fam = indi_to_famc(indi))) return NULL;
	node = CHIL(fam);
	found = FALSE;  /* until we find indi */
	while (node) {
		if (!found) {
			if (eqstr(nxref(indi), nval(node)))
				found = TRUE;
		} else {
			if (eqstr(ntag(node),"CHIL"))
				return key_to_record(rmvat(nval(node)));
		}
		node = nsibling(node);
	}
	return NULL;
}

RECORD
indi_to_next_sib (RECORD irec)
{
	return indi_to_next_sib_impl(nztop(irec));
}

/*======================================
 * node_to_tag -- Return a subtag of a node
 * (presumably top level, but not necessarily)
 *====================================*/
STRING node_to_tag (NODE node, STRING tag, INT len)
{
#if defined(DEADENDS)
	/* GEDCOM has no max, but MAXLINELEN is the longest line we support. */
	static char scratch[MAXLINELEN+1];
#else
	static char scratch[MAXGEDNAMELEN+1];
#endif
	STRING refn;
	if (!node) return NULL;
	if (!(node = find_tag(nchild(node), tag)))
		return NULL;
	refn = nval(node);
	if (len > (INT)sizeof(scratch)-1)
		len = sizeof(scratch)-1;
	llstrsets(scratch, len, uu8, refn);
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
record_to_date_place (RECORD record, STRING tag, STRING * date, STRING * plac
	, INT * count)
{
	NODE node=0;
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
NODE
record_to_first_event (RECORD record, CNSTRING tag)
{
	NODE node = nztop(record);
	if (!node) return NULL;
	return find_tag(nchild(node), tag);
}
/*==============================================
 * node_to_next_event -- Find next event after node
 *  node:  [IN]  start search after node
 *  tag:   [IN]  desired tag (eg, "BIRT")
 * Created: 2003-01-12 (Perry Rapp)
 *============================================*/
NODE
node_to_next_event (NODE node, CNSTRING tag)
{
	return find_tag(nsibling(node), tag);
}

/*===========================================
 * event_to_date_place  -- Find date & place
 *  node:  [IN]  node tree of event to describe
 *  date:  [OUT] value of first DATE line (optional)
 *  plac:  [OUT] value of first PLACE line (optional)
 *=========================================*/
void
event_to_date_place (NODE node, STRING * date, STRING * plac)
{
	INT count=0;
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
/*===========================================
 * event_to_string -- Convert event to string
 * Finds DATE & PLACE nodes, and prints a string
 * representation of them.
 *  node:  [IN]  node tree of event to describe
 *  ttm:   [IN]  translation table to use
 *  rfmt:  [IN]  reformatting info (may be NULL)
 *=========================================*/
STRING
event_to_string (NODE node, RFMT rfmt)
{
	static char scratch1[MAXLINELEN+1];
	STRING date, plac;
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
		llstrncpy(scratch1, date, sizeof(scratch1), uu8);
	} else if (plac && plac[0]) {
		llstrncpy(scratch1, plac, sizeof(scratch1), uu8);
	} else {
		return NULL;
	}
	return scratch1;
}

/*================================
 * show_node -- Show tree -- DEBUG
 *==============================*/
void
show_node (NODE node)
{
	if (!node) llwprintf("%s", "(NIL)");
	show_node_rec(0, node);
}
/*================================================
 * show_node_rec -- Recursive version of show_node
 *==============================================*/
void
show_node_rec (INT levl,
               NODE node)
{
	INT i;
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

/*========================
 * copy_node_subtree -- Copy tree
 *======================*/
NODE
copy_node_subtree (NODE node)
{
	return copy_nodes(node, TRUE, FALSE);
}

/*===============================================================
 * traverse_nodes -- Traverse nodes in tree while doing something
 * NODE node:    root of tree to traverse
 * func:         function to call at each node (returns FALSE to stop traversal)
 * param:        opaque pointer for client use, passed thru to callback
 *=============================================================*/
BOOLEAN
traverse_nodes (NODE node, BOOLEAN (*func)(NODE, VPTR), VPTR param)
{
	while (node) {
		if (!(*func)(node, param)) return FALSE;
		if (nchild(node)) {
			if (!traverse_nodes(nchild(node), func, param))
				return FALSE;
		}
		node = nsibling(node);
	}
	return TRUE;
}

/*==================================================
 * num_spouses_of_indi -- Returns number of spouses of person
 *================================================*/
INT
num_spouses_of_indi (NODE indi)
{
	INT nsp;
	if (!indi) return 0;
#if defined(DEADENDS)
	FORSPOUSES(indi, spouse, fam, nsp, database) ENDSPOUSES
#else
	FORSPOUSES(indi, spouse, fam, nsp) ENDSPOUSES
#endif
	return nsp;  /* don't include self*/
}

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
	destroy_list(alloc_block_list);
}

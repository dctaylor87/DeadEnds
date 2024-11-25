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
 * newrecs.c -- Handle source, event and other record types
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *   3.0.0 - 11 Sep 94    3.0.2 - 14 Apr 95
 *   3.0.3 - 17 Feb 96
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
#include "refnindex.h"
#include "gnode.h"
#include "readwrite.h"
#include "list.h"
#include "stringtable.h"
#include "options.h"

#include "zstr.h"
#include "translat.h"
#include "rfmt.h"
#include "sequence.h"
#include "xlat.h"
#include "ask.h"
#include "feedback.h"
#include "errors.h"
#include "liflines.h"
#include "messages.h"
#include "nodeutils.h"
//#include "readindex.h"
#include "database.h"
#include "xreffile.h"
#include "ll-addoperations.h"
#include "splitjoin.h"
#include "refns.h"
#include "locales.h"
#include "lloptions.h"
#include "editing.h"

#include "llpy-externs.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase

#include "llinesi.h"

/*********************************************
 * external/imported variables
 *********************************************/

//extern BTREE BTR;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static RecordIndexEl *edit_add_record(String recstr, String redt, String redtopt
	, char ntype, String cfrm);
static bool edit_record(RecordIndexEl *rec1, String idedt, int letr, String redt,
			   String redtopt,
			   bool (*val)(GNode *, String *, GNode *, Database *), String cfrm,
			   bool (*todbase)(GNode *, Database *),
			   String gdmsg, bool rfmt);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*================================================
 * edit_add_source -- Add source to database by editing
 *==============================================*/
RecordIndexEl *
edit_add_source (void)
{
	String str;

	str = getdeoptstr("SOURREC", _(qSdefsour));
	return edit_add_record(str, _(qSrredit), _(qSrreditopt), 'S', _(qScfradd));
}
/*==============================================
 * edit_add_event -- Add event to database by editing
 *============================================*/
RecordIndexEl *
edit_add_event (void)
{
	String str;

	str = getdeoptstr("EVENREC", _(qSdefeven));
	return edit_add_record(str, _(qSeredit), _(qSereditopt), 'E', _(qScfeadd));
}
/*====================================================
 * edit_add_other -- Add user record to database by editing
 *==================================================*/
RecordIndexEl *
edit_add_other (void)
{
	String str;

	str = getdeoptstr("OTHR", _(qSdefothr));
	return edit_add_record(str, _(qSxredit), _(qSxreditopt), 'X', _(qScfxadd));
}
/*================================================
 * edit_add_record -- Add record to database by editing
 *  recstr:  [IN] default record
 *  redt:    [IN] re-edit message
 *  redtopt: [IN] re-edit message for non critical faults
 *  ntype,   [IN] S, E, or X
 *  cfrm:    [IN] confirm message
 *==============================================*/
static RecordIndexEl *
edit_add_record (String recstr, String redt, String redtopt, char ntype, String cfrm)
{
	FILE *fp;
	GNode *node=0, *refn;
	String msg, key;
	bool emp;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);
	String (*getreffnc)(void) = NULL; /* get next internal key */
	bool (*todbasefnc)(GNode *, Database *) = NULL;  /* write record to dbase */
	
	/* set up functions according to type */
	if (ntype == 'S') {
		getreffnc = getsxref;
		todbasefnc = addOrUpdateSourceInDatabase;
	} else if (ntype == 'E') {
		getreffnc = getexref;
		todbasefnc = addOrUpdateEventInDatabase;
	} else { /* X */
		getreffnc = getxxref;
		todbasefnc = addOrUpdateOtherInDatabase;
	}

/* Create template for user to edit */
	if (!(fp = fopen(editfile, DEWRITETEXT))) {
		msg_error(_(qSnofopn), editfile);
		return false;
	}
	prefix_file_for_edit(fp);
	fprintf(fp, "%s\n", recstr);

/* Have user edit new record */
	fclose(fp);
	do_edit();
	while (true) {
		int cnt;
		node = file_to_node(editfile, ttmi, &msg, &emp);
		if (!node) {
			if (ask_yes_or_no_msg(msg, redt)) { /* yes, edit again */
				do_edit();
				continue;
			} 
			break;
		}
		cnt = resolve_refn_links(node);
		/* check validation & allow user to reedit if invalid */
		/* this is a showstopper, so alternative is to abort */
		if (!valid_node_type(node, ntype, &msg, NULL, currentDatabase)) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			}
			freeGNodes(node);
			node = NULL; /* fail out */
			break;
		}
		/* Allow user to reedit if desired if any refn links unresolved */
		/* this is not a showstopper, so alternative is to continue */
		if (cnt > 0) {
			char msgb[120];
			snprintf(msgb, sizeof(msgb)
				, get_unresolved_ref_error_string(cnt), cnt);
			if (ask_yes_or_no_msg(msgb, redtopt)) {
				write_node_to_editfile(node);
				do_edit();
				continue;
			}
		}
		break;
	}
	if (!node || !ask_yes_or_no(cfrm)) {
		if (node) freeGNodes(node);
		return NULL;
	}
	nxref(node) = strsave((String)(*getreffnc)());
	key = nxref(node);
	for (refn = nchild(node); refn; refn = nsibling(refn)) {
		if (eqstr("REFN", ntag(refn)) && nval(refn))
			addRefn(nval(refn), key, database);
	}
	(*todbasefnc)(node, database);
	return __llpy_key_to_record (key, NULL, currentDatabase);
}
/*=======================================
 * edit_source -- Edit source in database
 *=====================================*/
bool
edit_source (RecordIndexEl *rec, bool rfmt)
{
	return edit_record(rec, _(qSidredt), 'S', _(qSrredit), _(qSrreditopt),
			   valid_sour_tree, _(qScfrupt),
			   addOrUpdateSourceInDatabase, _(qSgdrmod), rfmt);
}
/*=====================================
 * edit_event -- Edit event in database
 *===================================*/
bool
edit_event (RecordIndexEl *rec, bool rfmt)
{
	return edit_record(rec, _(qSideedt), 'E', _(qSeredit), _(qSereditopt),
			   valid_even_tree, _(qScfeupt),
			   addOrUpdateEventInDatabase, _(qSgdemod), rfmt);
}
/*===========================================
 * edit_other -- Edit other record in database (eg, NOTE)
 *=========================================*/
bool
edit_other (RecordIndexEl *rec, bool rfmt)
{
	return edit_record(rec, _(qSidxedt), 'X', _(qSxredit), _(qSxreditopt),
			   valid_othr_tree, _(qScfxupt),
			   addOrUpdateOtherInDatabase, _(qSgdxmod), rfmt);
}
/*=======================================
 * edit_any_record -- Edit record of any type
 *=====================================*/
bool
edit_any_record (RecordIndexEl *rec, bool rfmt)
{
	ASSERT(rec);
	switch (recordType(rec->root)) {
	case GRPerson: return edit_indi(rec, rfmt);
	case GRFamily: return edit_family(rec, rfmt);
	case GRSource: return edit_source(rec, rfmt);
	case GREvent: return edit_event(rec, rfmt);
	case GROther: return edit_other(rec, rfmt);
	default: ASSERT(0); return false;
	}
}
/*========================================================
 * write_node_to_editfile - write all parts of gedcom node
 *  to a file for editing
 *======================================================*/
void
write_node_to_editfile (GNode *node)
{
	FILE *fp;
	XLAT ttmo = transl_get_predefined_xlat(MINED);

	ASSERT(fp = fopen(editfile, DEWRITETEXT));
	prefix_file_for_edit(fp);

	write_nodes(0, fp, ttmo, node,  true, true, true);
	fclose(fp);
}
/*=======================================
 * edit_record -- Edit record in database
 *  root1:   [IN]  record to edit (may be NULL)
 *  idedt:   [IN]  user id prompt
 *  letr:    [IN]  record type (E, S, or X)
 *  redt:    [IN]  reedit prompt displayed if hard error after editing
 *  redtopt: [IN]  reedit prompt displayed if soft error (unresolved links)
 *  val:     [IN]  callback to validate routine
 *  cfrm:    [IN]  confirmation msg string
 *  tag:     [IN]  tag (SOUR, EVEN, or NULL)
 *  todbase: [IN]  callback to write record to dbase
 *  gdmsg:   [IN]  success message
 *  rfmt:    [IN]  display reformatter
 *=====================================*/
static bool
edit_record(RecordIndexEl *rec1, String idedt, int letr, String redt,
	    String redtopt,
	    bool (*val)(GNode *, String *, GNode *, Database *), String cfrm,
	    bool (*todbase)(GNode *, Database *),
	    String gdmsg, bool rfmt)
{
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);
	String msg, key;
	bool emp;
	GNode *root0=0, *root1=0, *root2=0;
	GNode *refn1=0, *refn2=0, *refnn=0, *refn1n=0;
	GNode *body=0, *node=0;

/* Identify record if need be */
	if (!rec1) {
		rec1 = ask_for_record(idedt, letr);
	}
	root1 = nztop(rec1);
	if (!root1) {
		msg_error("%s", _(qSnosuchrec));
		return false;
	}

/* Have user edit record */
	annotateWithSupplemental(root1, rfmt, database);
	write_node_to_editfile(root1);
	resolve_refn_links(root1);

	do_edit();

	while (true) {
		int cnt;
		root2 = file_to_node(editfile, ttmi, &msg, &emp);
		if (!root2) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			}
			break;
		}
		cnt = resolve_refn_links(root2);
		/* check validation & allow user to reedit if invalid */
		/* this is a showstopper, so alternative is to abort */
		if (!(*val)(root2, &msg, root1, database)) {
			if (ask_yes_or_no_msg(msg, redt)) {
				do_edit();
				continue;
			}
			freeGNodes(root2);
			root2 = NULL;
			break;
		}
		/* Allow user to reedit if desired if any refn links unresolved */
		/* this is not a showstopper, so alternative is to continue */
		if (cnt > 0) {
			char msgb[120];
			snprintf(msgb, sizeof(msgb)
				, get_unresolved_ref_error_string(cnt), cnt);
			if (ask_yes_or_no_msg(msgb, redtopt)) {
				write_node_to_editfile(root2);
				do_edit();
				continue;
			}
		}
		break;
	}

/* If error or no change or user backs out return */
	if (!root2) return false;
	if (equalTree(root1, root2) || !ask_yes_or_no(cfrm)) {
		freeGNodes(root2);
		return false;
	}

/* Prepare to change database */

	/* Move root1 data into root0 & save refns */
	splitOther(root1, &refn1, &body);
	root0 = copyNode(root1);
	joinOther(root0, NULL, body);
	/* delete root0 tree & root1 node (root1 is solitary node) */
	freeGNodes(root0); root0 = 0;
	freeGNodes(root1); root1 = 0;
	/* now copy root2 node into root1, then root2 tree under it */
	root1 = copyNode(root2);
	splitOther(root2, &refn2, &body);
	refnn = copyNodes(refn2, true, true);
	joinOther(root1, refn2, body);
	/* now root2 is solitary node, delete it */
	free_node(root2,"edit_record"); root2 = 0;

/* Change the database */

	(*todbase)(root1, database);
	key = nxref(root1);
	/* remove deleted refns & add new ones */
	classifyNodes(&refn1, &refnn, &refn1n);
	for (node = refn1; node; node = nsibling(node))
		if (nval(node)) remove_refn(nval(node), key);
	for (node = refnn; node; node = nsibling(node))
		if (nval(node)) addRefn(nval(node), key, database);
	freeGNodes(refn1);
	freeGNodes(refnn);
	freeGNodes(refn1n);
	msg_info("%s", gdmsg);
	return true;
}

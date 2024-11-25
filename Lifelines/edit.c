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
 * edit.c -- Edit person or family record
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 12 Dec 94
 *   3.0.3 - 15 Feb 96
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
#include "stringtable.h"
#include "list.h"
#include "options.h"

#include "zstr.h"
#include "translat.h"
#include "refnindex.h"
#include "recordindex.h"
#include "rfmt.h"
#include "sequence.h"
#include "xlat.h"
#include "ask.h"
#include "feedback.h"
#include "llinesi.h"
#include "errors.h"
#include "liflines.h"
#include "messages.h"
#include "choose.h"
#include "readwrite.h"
#include "lineage.h"
#include "gnode.h"
#include "replace.h"
#include "editing.h"
#include "refns.h"
#include "nodeutils.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=====================================
 * edit_indi -- Edit person in database
 * (with user interaction)
 * returns TRUE if user makes changes (& saves them)
 *===================================*/
bool
edit_indi (RecordIndexEl *irec1, bool rfmt)
{
	GNode *indi1, *indi2=0;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);

/* Identify indi if necessary */
	if (!irec1 && !(irec1 = ask_for_indi(_(qSidpedt), NOASK1)))
		return false;
	indi1 = nztop(irec1);

/* Prepare file for user to edit */
#if !defined(DEADENDS)
	nodechk(indi1, "edit_indi");
#endif
	write_indi_to_file_for_edit(indi1, editfile, rfmt, currentDatabase);

/* Have user edit file */
	do_edit();

	while (true) {
		int cnt;
		String msg;
		bool emp;
		indi2 = file_to_node(editfile, ttmi, &msg, &emp);
		if (!indi2) {
			if (ask_yes_or_no_msg(msg, _(qSiredit))) {
				do_edit();
				continue;
			} 
			break;
		}
		cnt = resolve_refn_links(indi2);
		/* validate for showstopper errors */
		if (!valid_indi_tree(indi2, &msg, indi1, currentDatabase)) {
			/* if fail a showstopper error, must reedit or abort */
			if (ask_yes_or_no_msg(msg, _(qSiredit))) {
				do_edit();
				continue;
			}
			freeGNodes(indi2);
			indi2 = NULL;
			break;
		}
		/* Allow user to reedit if desired if any refn links unresolved */
		/* this is not a showstopper, so alternative is to continue */
		if (cnt > 0) {
			char msgb[120];
			snprintf(msgb, sizeof(msgb)
				, get_unresolved_ref_error_string(cnt), cnt);
			if (ask_yes_or_no_msg(msgb, _(qSireditopt))) {
				write_indi_to_file_for_edit(indi2, editfile, rfmt, currentDatabase);
				do_edit();
				continue;
			}
		}
		break;
	}

/* Editing done; see if database changes */

	if (!indi2) return false;
	if (equalTree(indi1, indi2) || !ask_yes_or_no(_(qScfpupt))) {
		freeGNodes(indi2);
		return false;
	}

/* Move new data (in indi2 children) into existing indi1 tree */
	replace_indi(indi1, indi2, currentDatabase);

/* Note in change history */
	history_record_change(irec1);
	
	msg_status(_(qSgdpmod), personToName(indi1, 35));
	return true;
}
/*====================================
 * edit_fam -- Edit family in database
 * (with user interaction)
 *==================================*/
bool
edit_family (RecordIndexEl *frec1, bool rfmt)
{
	GNode *fam1=0, *fam2=0;
	RecordIndexEl *irec=0;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);
	String msg;
	bool changed = false;

/* Identify family if necessary */
	if (!frec1) {
		irec = ask_for_indi(_(qSidspse), NOASK1);
		if (!irec) return false;
		if (!FAMS(nztop(irec))) {
			msg_error("%s", _(qSntprnt));
			goto end_edit_fam;
		} 
		frec1 = chooseFamily(irec, _(qSparadox), _(qSidfbys), true);
		if (!frec1) return false; 
	}
	fam1 = nztop(frec1);

/* Prepare file for user to edit */
	write_fam_to_file_for_edit(fam1, editfile, rfmt, currentDatabase);

/* Have user edit record */
	do_edit();
	while (true) {
		int cnt;
		bool emp;
		fam2 = file_to_node(editfile, ttmi, &msg, &emp);
		if (!fam2) {
			if (ask_yes_or_no_msg(msg, _(qSfredit))) {
				do_edit();
				continue;
			}
			break;
		}
		cnt = resolve_refn_links(fam2);
		/* check validation & allow user to reedit if invalid */
		/* this is a showstopper, so alternative is to abort */
		if (!valid_fam_tree(fam2, &msg, fam1, currentDatabase)) {
			if (ask_yes_or_no_msg(msg, _(qSfredit))) {
				do_edit();
				continue;
			}
			freeGNodes(fam2);
			fam2 = NULL;
			break;
		}
		/* Allow user to reedit if desired if any refn links unresolved */
		/* this is not a showstopper, so alternative is to continue */
		if (cnt > 0) {
			char msgb[120];
			snprintf(msgb, sizeof(msgb)
				, get_unresolved_ref_error_string(cnt), cnt);
			if (ask_yes_or_no_msg(msgb, _(qSfreditopt))) {
				write_fam_to_file_for_edit(fam2, editfile, rfmt, currentDatabase);
				do_edit();
				continue;
			}
		}
		break;
	}

/* If error or user backs out return */

	if (!fam2) return false;
	if (equalTree(fam1, fam2) || !ask_yes_or_no(_(qScffupt)))
		goto end_edit_fam;

/* Move new data (in fam2 children) into existing fam1 tree */
	replace_fam(fam1, fam2, currentDatabase);
	fam2 = NULL;

	msg_status("%s", _(qSgdfmod));
	changed = true;

end_edit_fam:
	if (fam2)
		freeGNodes(fam2); 
	return changed;
}

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
/*==============================================================
 * lbrowse.c -- Handle list browse mode
 * NB: Part of curses GUI version
 * Copyright (c) 1993-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 09 Oct 94    3.0.2 - 30 Dec 94
 *   3.0.3 - 20 Jan 96
 *============================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "denls.h"
#include "sys_inc.h"

#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "rfmt.h"
#include "refnindex.h"
#include "sequence.h"
#include "ask.h"
#include "feedback.h"
#include "errors.h"
#include "liflines.h"
#include "menuitem.h"
#include "messages.h"
#include "sequence.h"
#include "gnode.h"
#include "database.h"
#include "recordindex.h"
#include "ll-sequence.h"
#include "browse.h"
#include "llpy-externs.h"

/* everything in this file assumes we are dealing with the current database */
//#define database	currentDatabase

#include "llinesi.h"
#include "screen.h"

#define VIEWABLE 13

extern Sequence *current_seq;

static void name_the_list(Sequence *seq);

/*=======================================
 * browse_list -- Handle list browse mode
 *=====================================*/
int
browse_list (RecordIndexEl **prec1, RecordIndexEl **prec2, Sequence **pseq)
{
	int c, top, cur, mark, len, tmp, rc;
	CString key, name;
	String lname="";
	RecordIndexEl *rec=0;
	Sequence *seq, *newseq;
	
	ASSERT(prec1);
	ASSERT(!*prec1);
	ASSERT(prec2);
	ASSERT(!*prec2);
	ASSERT(pseq);
	ASSERT(*pseq);
	seq = *pseq;
	*pseq = 0;
	current_seq = NULL;
	if ((len = lengthSequence(seq)) <= 0)
		return  BROWSE_QUIT;
	top = cur = 0;
	mark =  -1;
#if !defined(DEADENDS)	 /* DEADENDS always fills in names for a person sequence */
	calc_indiseq_names(seq); /* ensure we have names */
#endif
	current_seq = seq;

	while (true) {
		element_indiseq(seq, cur, &key, &name);
		rec = __llpy_key_to_record(key, NULL, seq->database);
		switch (c = list_browse(seq, top, &cur, mark)) {
		case 'j':        /* Move down line */
		case CMD_KY_DN:
			if (cur >= len - 1) {
				message("%s", _(qSlstbot));
				break;
			}
			cur++;
			if (cur >= top + VIEWABLE) top++;
			break;
		case 'd':        /* Move down one page */
		case CMD_KY_PGDN:
			if (top + VIEWABLE >= len-1) {
				message("%s", _(qSlstbot));
				break;
			}
			cur += VIEWABLE;
			if (cur > len-1)
				cur = len-1;
			top += VIEWABLE;
			break;
		case 'D':        /* Move down several pages */
		case CMD_KY_SHPGDN:
			if (top + VIEWABLE >= len-1) {
				message("%s", _(qSlstbot));
				break;
			}
			tmp = (len)/10;
			if (tmp < VIEWABLE*2) tmp = VIEWABLE*2;
			if (tmp > len - VIEWABLE - top)
				tmp = len - VIEWABLE - top;
			top += tmp;
			cur += tmp;
			if (cur > len-1)
				cur = len-1;
			break;
		case '$':        /* jump to end of list */
		case CMD_KY_END:
			top = len - VIEWABLE;
			if (top < 0)
				top = 0;
			cur = len-1;
			break;
		case 'k':        /* Move up line */
		case CMD_KY_UP:
			if (cur <= 0) {
				message("%s", _(qSlsttop));
				break;
			}
			cur--;
			if (cur + 1 == top) top--;
			break;
		case 'u':        /* Move up one page */
		case CMD_KY_PGUP:
			if (top <= 0) {
				message("%s", _(qSlsttop));
				break;
			}
			tmp = VIEWABLE;
			if (tmp > top) tmp = top;
			cur -= tmp;
			top -= tmp;
			break;
		case 'U':        /* Move up several pages */
		case CMD_KY_SHPGUP:
			if (top <= 0) {
				message("%s", _(qSlsttop));
				break;
			}
			tmp = (len)/10;
			if (tmp < VIEWABLE*2) tmp = VIEWABLE*2;
			if (tmp > top) tmp = top;
			cur -= tmp;
			top -= tmp;
			break;
		case '^':        /* jump to top of list */
		case CMD_KY_HOME:
			top = cur = 0;
			break;
		case 'e':        /* Edit current person */
			edit_any_record(rec, false);
			if ((len = lengthSequence(seq)) <= 0) {
				remove_browse_list(lname, seq);
				current_seq = NULL;
				lname = NULL;
				return BROWSE_QUIT;
			}
			if (cur >= len) cur = len - 1;
			break;
		case 'i':        /* Browse current person */
		case CMD_KY_ENTER:
			*prec1 = rec;
			if (current_seq)
				remove_indiseq(current_seq);
			current_seq = NULL;
			return BROWSE_UNK;
		case 'm':        /* Mark current person */
			mark = (cur == mark) ? -1: cur;
			break;
		case 'r':        /* Remove person from list */
			if (len <= 1) {
				if (current_seq)
					remove_indiseq(current_seq);
				current_seq = NULL;
				return BROWSE_QUIT;
			}
			removeFromSequenceByIndex (seq, cur);
			len--;
			if (mark == cur) mark = -1;
			if (mark > cur) mark--;
			if (cur == len)
				cur--;
			if (cur < top) top = cur;
			break;
		case 't':        /* Enter tandem mode */
		{
			RecordIndexEl *cand1=0, *cand2=0;
			if (mark == -1 || cur == mark) {
				msg_error("%s", _(qSmrkrec));
				break;
			}
			cand2 = rec;
			element_indiseq(seq, mark, &key, &name);
			cand1 = __llpy_key_to_record(key, NULL, seq->database);
			if (recordType(cand1->root)==GRPerson && recordType(cand2->root)==GRPerson) {
				current_seq = NULL;
				*prec1 = cand1;
				*prec2 = cand2;
				return BROWSE_TAND;
			} else if (recordType(cand1->root)==GRFamily && recordType(cand2->root)==GRFamily) {
				current_seq = NULL;
				*prec1 = cand1;
				*prec2 = cand2;
				return BROWSE_2FAM;
			} else {
				msg_error("%s", _("Tandem browse only compatible with persons or families."));
				break;
			}
		}
		case 'b':        /* Browse new persons */
			newseq = ask_for_indiseq(_(qSidplst), 'I', &rc);
			/* If no new indiseq was created, then the previous indiseq is still in effect. */
			if (!newseq) break;
			/* Otherwise, if a new indiseq was created, then the previous indiseq must be freed. */
			if (current_seq) {
				remove_indiseq(current_seq);
			}
			current_seq = seq = newseq;
			element_indiseq(seq, 0, &key, &name);
			if ((len = lengthSequence(seq)) == 1) {
				*prec1 = keyToPersonRecord(key, seq->database);
				remove_indiseq(newseq);
				current_seq = NULL;
				return BROWSE_INDI;
			}
			top = cur = 0;
			mark = -1;
			break;
		case 'a':        /* Add persons to current list */
		{
			CString skey=0, snam=0;
			newseq = ask_for_indiseq(_(qSlstpad), 'I', &rc);
			if (!newseq) {
				message("%s", _(qSlstnad));
				break;
			}
			FORSEQUENCE(newseq, el, i)
				skey = element_skey(el);
				snam = element_name(el);
				append_indiseq_null(seq, strsave(skey), snam, false, true);
			ENDSEQUENCE
			namesort_indiseq(seq);
			cur = top = 0;
			mark = -1;
			len = lengthSequence(seq);
			remove_indiseq(newseq);
			message("%s", _(qSlstnew));
			break;
		}
		case 'n':        /* Name this list */
			name_the_list(seq);
			break;
		case 'x':        /* Swap current with marked */
			if (mark == -1) break;
			tmp = mark;
			mark = cur;
			cur = tmp;
			if (cur < top) top = cur;
			if (cur > top + VIEWABLE - 1) top = cur;
			break;
		case 'q':        /* Return to main menu */
			if (current_seq)
				remove_indiseq(current_seq);
			current_seq = NULL;
			return BROWSE_QUIT;
		}
	}
}
/*=======================================
 * name_the_list -- Assign a name to sequence
 *=====================================*/
static void
name_the_list (Sequence *seq)
{
	char name[MAXPATHLEN];
	if (!ask_for_string(_(qSlstwht), _(qSasknam), name, sizeof(name))
		|| !name[0]) {
		msg_error("%s", _(qSlstnon));
		return;
	}
	/* TODO: Should just let the browse list addref the indiseq */
	add_browse_list(strsave(name), copySequence(seq));
	msg_info(_(qSlstnam), name);
}

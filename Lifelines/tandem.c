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
 * tandem.c -- LifeLines tandem browsing operations
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   2.3.6 - 01 Nov 93    3.0.0 - 05 May 94
 *   3.0.2 - 29 Dec 94
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
#include "rfmt.h"
#include "refnindex.h"
#include "sequence.h"
#include "errors.h"
#include "ask.h"
#include "choose.h"
#include "feedback.h"
#include "llinesi.h"
#include "liflines.h"
#include "menuitem.h"
#include "messages.h"
#include "screen.h"
#include "lineage.h"
#include "gnode.h"
//#include "readindex.h"
#include "database.h"
#include "ll-addoperations.h"
#include "llpy-externs.h"

/* everything in this file assumes we are dealing with the current database */
//#define database	currentDatabase

/*********************************************
 * local function prototypes
 *********************************************/

static bool handle_tandem_scroll_cmds(int c);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=============================================
 * browse_tandem -- Two person browse operation
 *===========================================*/
int browse_tandem (GNode **prec1, GNode **prec2, Sequence **pseq)
{
	GNode *current1, *current2;
	CString nkey1p, nkey2p;
	int modep;
	GNode *tmp=0;
	CString key, name;
	Sequence *seq;
	int c, rc;
	bool reuse=false;
	static int mode = 'n';

	ASSERT(prec1);
	ASSERT(*prec1);
	ASSERT((*prec1)->parent);
	ASSERT(nztype(*prec1)==GRPerson);
	ASSERT(prec2);
	ASSERT(*prec2);
	ASSERT((*prec2)->parent);
	ASSERT(nztype(*prec2)==GRPerson);
	ASSERT(! *pseq);
	current1 = *prec1;
	current2 = *prec2;

	*prec1 = 0;
	*prec2 = 0;
	*pseq = 0;

	show_reset_scroll();
	nkey1p = NULL; /* force redraw */
	nkey2p = NULL;
	modep = mode;

	while (true) {
		if (! nkey1p || nestr (nzkey(current1), nkey1p) ||
		    ! nkey2p || nestr(nzkey(current2), nkey2p) ||
		    (mode != modep)) {
			show_reset_scroll();
		}
		display_2indi(current1, current2, mode);
		c = interact_2indi();
		/* last keynum & mode, so can tell if changed */
		nkey1p = nzkey(current1);
		nkey2p = nzkey(current2);
		modep = mode;
		if (handle_menu_cmds(c, &reuse))
			continue;
		if (handle_indi_mode_cmds(c, &mode))
			continue;
		if (handle_tandem_scroll_cmds(c))
			continue;
		switch (c)
		{
		case CMD_EDIT: 	/* edit top person */
			edit_indi(current1, false);
			break;
		case CMD_TOP: 	/* browse top person */
			*prec1 = current1;
			return BROWSE_INDI;
		case CMD_FATHER: 	/* browse top person's father */
			if ((tmp = chooseFather(current1, NULL, _(qSnofath),
				_(qSidhbrs), NOASK1, currentDatabase)) != 0) {
				current1 = tmp;
			}
			break;
		case CMD_MOTHER: 	/* browse top person's mother */
			if ((tmp = chooseMother(current1, NULL, _(qSnomoth),
				_(qSidwbrs), NOASK1, currentDatabase)) != 0) {
				current1 = tmp;
			}
			break;
		case CMD_SPOUSE: 	/* browse top person's spouse/s */
			if ((tmp = chooseSpouse(current1, _(qSnospse), _(qSidsbrs), currentDatabase)) != 0)
				current1 = tmp;
			break;
		case CMD_CHILDREN: 	/* browse top person's children */
			if ((tmp = chooseChild(current1, NULL, _(qSnocofp),
				_(qSidcbrs), NOASK1, currentDatabase)) != 0)
				current1 = tmp;
			break;
		case CMD_MERGE_BOTTOM_TO_TOP: 	/* merge two persons */
			if ((tmp = merge_two_indis(nztop(current2), nztop(current1), true))) {
				*prec1 = tmp;
				return BROWSE_INDI;
			}
			break;
		case CMD_COPY_TOP_TO_BOTTOM: 	/* copy top person to bottom */
			current2 = current1;
			break;
		case CMD_SWAPTOPBOTTOM: 	/* swap two persons */
			tmp = current1;
			current1 = current2;
			current2 = tmp;
			break;
		case CMD_ADDFAMILY: 	/* make two persons parents in family */
			tmp = add_family_by_edit(current1, current2, NULL, false);
			if (tmp) {
				*prec1 = tmp;
				return BROWSE_FAM;
			}
			break;
		case CMD_BROWSE: 	/* browse to new person list */
			seq = ask_for_indiseq(_(qSidplst), 'I', &rc, currentDatabase);
			if (!seq) break;
			if (lengthSequence(seq) == 1) {
				elementFromSequence(seq, 0, &key, &name);
				*prec1 = keyToPerson(key, seq->index);
				deleteSequence(seq);
				return BROWSE_INDI;
			}
			*pseq = seq;
			return BROWSE_LIST;
			break;
		case CMD_DEPTH_DOWN:       /* decrease pedigree depth */
			pedigree_increase_generations(-1);
			break;
		case CMD_DEPTH_UP:      /* increase pedigree depth */
			pedigree_increase_generations(+1);
			break;
		case CMD_QUIT:
			return BROWSE_QUIT;
		}
	}
}
/*==================================================
 * browse_2fam -- Handle two family browse operation
 *================================================*/
int browse_2fam (GNode **prec1, GNode **prec2, Sequence **pseq)
{
	GNode *current1, *current2;
	GNode *tmp, *tmp2;
	CString nkey1p, nkey2p;
	int modep;
	int c;
	bool reuse=false;
	static int mode = 'n';

	ASSERT(prec1);
	ASSERT(*prec1);
	ASSERT(nztype(*prec1)==GRFamily);
	ASSERT(prec2);
	ASSERT(*prec2);
	ASSERT(nztype(*prec2)==GRPerson);
	ASSERT(! *pseq);
	current1 = *prec1;
	current2 = *prec2;

	*prec1 = 0;
	*prec2 = 0;
	*pseq = 0;

	show_reset_scroll();
	nkey1p = NULL; /* force redraw */
	nkey2p = NULL;
	modep = mode;

	while (true) {
		if (! nkey1p || nestr (nzkey(current1), nkey1p) ||
		    ! nkey2p || nestr (nzkey(current2), nkey2p) ||
		    (mode != modep)) {
			show_reset_scroll();
		}
		display_2fam(current1, current2, mode);
		c = interact_2fam();
		/* last keynum & mode, so can tell if changed */
		nkey1p = nzkey(current1);
		nkey2p = nzkey(current2);
		modep = mode;
		if (handle_menu_cmds(c, &reuse))
			continue;
		if (handle_fam_mode_cmds(c, &mode))
			continue;
		if (handle_tandem_scroll_cmds(c))
			continue;
		switch (c)
		{
		case CMD_EDIT:	/* edit top fam */
			edit_family(current1, false);
			break;
		case CMD_TOP:	/* browse top fam */
			*prec1 = current1;
			return BROWSE_FAM;
		case CMD_BOTTOM:	/* browse bottom fam */
			*prec1 = current2;
			return BROWSE_FAM;
		case CMD_BOTH_FATHERS:	/* browse to husbs/faths */
			{
				GNode *fam1=0, *fam2=0;
				GNode *node1;
				GNode *node2;

				node1 = familyToHusband (nztop(current1), currentDatabase->recordIndex);
				node2 = familyToHusband (nztop(current2), currentDatabase->recordIndex);
#if defined(DEADENDS)
				for (fam1 = node1; fam1; fam1 = fam1->parent)
				  ;
				for (fam2 = node2; fam2; fam2 = fam2->parent)
				  ;
#else
				fam1 = _llpy_node_to_record (node1, currentDatabase);
				fam2 = _llpy_node_to_record (node2, currentDatabase);
#endif
				if (fam1 && fam2) {
				  *prec1 = fam1;
				  *prec2 = fam2;
				  return BROWSE_TAND;
				}
			}
			msg_error("%s", _(qStwohsb));
			break;
		case CMD_BOTH_MOTHERS:	/* browse to wives/moths */
			if ((tmp = chooseMother(NULL, current1, _(qSnowife),
				_(qSid1wbr), NOASK1, currentDatabase)) != 0) {
				if ((tmp2 = chooseMother(NULL, current2, _(qSnowife), 
					_(qSid2wbr), NOASK1, currentDatabase)) != 0) {
					*prec1 = tmp;
					*prec2 = tmp2;
					return BROWSE_TAND;
				}
			}
			msg_error("%s", _(qStwowif));
			break;
		case CMD_MERGE_BOTTOM_TO_TOP:	/* merge two fams */
			if ((tmp = merge_two_fams(nztop(current2), nztop(current1))) != 0) {
				*prec1 = tmp;
				return BROWSE_FAM;
			}
			break;
		case CMD_SWAPTOPBOTTOM:	/* swap two fams */
			tmp = current1;
			current1 = current2;
			current2 = tmp;
			break;
		case CMD_TOGGLE_CHILDNUMS:       /* toggle children numbers */
			show_childnumbers();
			break;
		case CMD_QUIT:	/* Return to main menu */
			return BROWSE_QUIT;
		}
	}
}
/*======================================================
 * handle_tandem_scroll_cmds -- Handle tandem scrolling
 * Created: 2001/02/04, Perry Rapp
 *====================================================*/
static bool
handle_tandem_scroll_cmds (int c)
{
	switch(c) {
	case CMD_SCROLL_TOP_UP: show_scroll(-1); return true;
	case CMD_SCROLL_TOP_DOWN: show_scroll(+1); return true;
	case CMD_SCROLL_BOTTOM_UP: show_scroll2(-1); return true;
	case CMD_SCROLL_BOTTOM_DOWN: show_scroll2(+1); return true;
	case CMD_SCROLL_BOTH_UP: show_scroll(-1); show_scroll2(-1); return true;
	case CMD_SCROLL_BOTH_DOWN: show_scroll(+1); show_scroll2(+1); return true;
	}
	return false;
}

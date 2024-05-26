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
/*=================================================================
 * swap.c -- Swaps two children of family or two families of person
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 27 Sep 93
 *   2.3.6 - 01 Nov 93    3.0.0 - 23 Sep 94
 *   3.0.2 - 02 Dec 94
 *===============================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "llnls.h"
#include "options.h"

#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "refnindex.h"
#include "gnode.h"
#include "rfmt.h"
#include "editing.h"
#include "sequence.h"
#include "uiprompts.h"
#include "choose.h"
#include "feedback.h"
#include "errors.h"
#include "liflines.h"
#include "messages.h"
#include "splitjoin.h"
#include "remove.h"
#include "ll-addoperations.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase

#include "llinesi.h"

/*********************************************
 * local function prototypes
 *********************************************/
static int child_index(GNode *child, GNode *fam);
static bool confirm_and_swap_children_impl(GNode *fam, GNode *one, GNode *two);
static void swap_children_impl(GNode *fam, GNode *one, GNode *two);

/*=============================================
 * swap_children -- Swap two children in family
 *  prnt: [IN]  parent (may be NULL)
 *  fam:  [IN]  family (may be NULL)
 *===========================================*/
bool
swap_children (RecordIndexEl *prnt, RecordIndexEl *frec)
{
	GNode *fam, *chil, *one, *two;
	int nfam, nchil;

/* Identify parent if need be */
	if (frec) goto gotfam;
	if (!prnt) prnt = ask_for_indi(_(qSidcswp), NOASK1);
	if (!prnt) return false;
	nfam = num_families(nztop(prnt));
	if (nfam <= 0) {
		msg_error("%s", _(qSntchld));
		return false;
	}

/* Identify family if need be */
	if (nfam == 1) {
		frec = key_to_frecord(rmvat(nval(FAMS(nztop(prnt)))));
		goto gotfam;
	}
	if (!(frec = choose_family(prnt, _(qSntprnt), _(qSidfbys), true)))
		return false;
gotfam:
	fam = nztop(frec);
	nchil = num_children(fam);
	if (nchil < 2) {
		msg_error("%s", _(qSless2c));
		return false;
	}

	if (nchil == 2) {
		/* swap the two existing ones */
		one = CHIL(fam);
		two = nsibling(one);
	} else {
		RecordIndexEl *chil1, *chil2;
		STRING key1, key2;
		/* Identify children to swap */
		chil1 = choose_child(NULL, frec, "e", _(qSid1csw), NOASK1);
		if (!chil1) return false;
		chil2 = choose_child(NULL, frec, "e", _(qSid2csw), NOASK1);
		if (!chil2) return false;
		if (chil1 == chil2) return false;
		key1 = nxref(nztop(chil1));
		key2 = nxref(nztop(chil2));

		/* loop through children & find the ones chosen */
		ASSERT(chil = CHIL(fam));
		one = two = NULL;
		for (;  chil;  chil = nsibling(chil)) {
			if (eqstr(ntag(chil), "CHIL")) {
				if (eqstr(key1, nval(chil))) one = chil;
				if (eqstr(key2, nval(chil))) two = chil;
			}
		}
	}

	if (!confirm_and_swap_children_impl(fam, one, two))
		return false;
	msg_info("%s", _(qSokcswp));
	return true;
}
/*=============================================
 * confirm_and_swap_children_impl -- Swap input children in input family
 *  fam:     [in] family of interest
 *  one,two: [in] children to swap
 * inputs assumed valie
 * confirm then call worker
 *===========================================*/
static bool
confirm_and_swap_children_impl (GNode *fam, GNode *one, GNode *two)
{
	if (!ask_yes_or_no(_(qScfchswp)))
		return false;

	swap_children_impl(fam, one, two);
	return true;
}
/*=============================================
 * swap_children_impl -- Swap input children in input family
 *  fam:     [in] family of interest
 *  one,two: [in] children to swap
 * all inputs assumed valid - no user feedback here
 *===========================================*/
static void
swap_children_impl (GNode *fam, GNode *one, GNode *two)
{
	STRING str;
	GNode *tmp;
	ASSERT(one);
	ASSERT(two);
   /* Swap CHIL nodes and update database */
	str = nval(one);
	nval(one) = nval(two);
	nval(two) = str;
	tmp = nchild(one);
	nchild(one) = nchild(two);
	nchild(two) = tmp;
	fam_to_dbase(fam);
}
/*=============================================
 * reorder_child -- Reorder one child in family
 *  prnt: [IN]  parent (may be NULL)
 *  fam:  [IN]  family (may be NULL)
 *  rftm: [IN]  person formatting for prompts
 *===========================================*/
bool
reorder_child (RecordIndexEl *prnt, RecordIndexEl *frec, bool rfmt)
{
	int nfam, nchil;
	int prevorder, i;
	GNode *fam, *child;

/* Identify parent if need be */
	if (frec) goto gotfam;
	if (!prnt) prnt = ask_for_indi(_(qSidcswp), NOASK1);
	if (!prnt) return false;
	nfam = num_families(nztop(prnt));
	if (nfam <= 0) {
		msg_error("%s", _(qSntchld));
		return false;
	}

/* Identify family if need be */
	if (nfam == 1) {
		frec = key_to_frecord(rmvat(nval(FAMS(nztop(prnt)))));
		goto gotfam;
	}
	if (!(frec = choose_family(prnt, _(qSntprnt), _(qSidfbys), true))) 
		return false;
gotfam:
	fam = nztop(frec);
	nchil = num_children(fam);
	if (nchil < 2) {
		msg_error("%s", _(qSless2c));
		return false;
	}

	if (nchil == 2) {
		/* swap the two existing ones */
		GNode *one = CHIL(fam);
		GNode *two = nsibling(one);
		if (!confirm_and_swap_children_impl(fam, one, two))
			return false;
		message("%s", _(qSokcswp));
		return true;
	}

	/* Identify children to swap */
	child = nztop(choose_child(NULL, frec, "e", _(qSidcrdr), NOASK1));
	if (!child) return false;

	prevorder = child_index(child, fam);

	/* first remove child, so can list others & add back */
	remove_child(child, fam, currentDatabase);

	i = ask_child_order(fam, ALWAYS_PROMPT, rfmt);
	if (i == -1 || !ask_yes_or_no(_(qScfchswp))) {
		/* must put child back if cancel */
		add_child_to_fam(child, fam, prevorder);
		return false;
	}

/* Add FAMC node to child */

	add_child_to_fam(child, fam, i);

	fam_to_dbase(fam);
	return true;
}
/*=============================================
 * child_index -- What is child's birth index in this family ?
 *  child:  [in] child of interest
 *  fam:    [in] family of interest
 *===========================================*/
static int
child_index (GNode *child, GNode *fam)
{
	int j;
	GNode *husb, *wife, *chil, *rest, *fref;
	GNode *node;
	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
	node = chil;
	j = 0;
	while (node && !eqstr(nval(node), nxref(child))) {
		++j;
		node = nsibling(node);
	}
	join_fam(fam, fref, husb, wife, chil, rest);
	return child ? j : -1;
}
/*=============================================
 * swap_families -- Swap two families of person
 *  indi:  [in] person
 *  prompt for indi if NULL
 *  prompt for families if person chosen has >2
 *  Ask for yes/no confirm
 *===========================================*/
bool
swap_families (RecordIndexEl *irec)
{
	GNode *indi, *fams, *one, *two, *tmp;
	int nfam;
	STRING str;

/* Find person and assure has >= 2 families */
	if (!irec) irec = ask_for_indi(_(qSidfswp), NOASK1);
	if (!irec) return false;
	indi = nztop(irec);
	if (!(fams = FAMS(indi))) {
		msg_error("%s", _(qSntprnt));
		return false;
	}
	nfam = num_families(indi);
	if (nfam < 2) {
		msg_error("%s", _(qSless2f));
		return false;
	}

/* Find families to swap */
	ASSERT(fams = FAMS(indi));
	if (nfam == 2) {
		/* swap the two existing ones */
		one = fams;
		two = nsibling(fams);
	} else {
		GNode *fam1, *fam2;
		STRING key1, key2;
		/* prompt for families */
		fam1 = nztop(choose_family(irec, _(qSparadox), _(qSid1fsw), true));
		if (!fam1) return false;
		fam2 = nztop(choose_family(irec, _(qSparadox), _(qSid2fsw), true));
		if (!fam2) return false;
		if (fam1 == fam2) return false;
		key1 = nxref(fam1);
		key2 = nxref(fam2);
		/* loop through families & find the ones chosen */
		one = two = NULL;
		for (;  fams;  fams = nsibling(fams)) {
			if (eqstr(key1, nval(fams))) one = fams;
			if (eqstr(key2, nval(fams))) two = fams;
		}
	}

	ASSERT(one);
	ASSERT(two);
	ASSERT(eqstr(ntag(one), "FAMS"));
	ASSERT(eqstr(ntag(two), "FAMS"));

/* Ask user to confirm */
	if (!ask_yes_or_no(_(qScffswp)))
		return false;

/* Swap FAMS nodes and update database */
	str = nval(one);
	nval(one) = nval(two);
	nval(two) = str;
	tmp = nchild(one);
	nchild(one) = nchild(two);
	nchild(two) = tmp;
	indi_to_dbase(indi);
	msg_info("%s", _(qSokfswp));
	return true;
}

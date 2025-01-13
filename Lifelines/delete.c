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
 * delete.c -- Removes person and family records from database 
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 15 Aug 93
 *   3.0.0 - 30 Jun 94    3.0.2 - 10 Dec 94
 *   3.0.3 - 21 Jan 96
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
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
#include "feedback.h"
#include "refnindex.h"
#include "gnode.h"
#include "xlat.h"
#include "readwrite.h"
#include "recordindex.h"

#include "rfmt.h"
#include "sequence.h"
#include "ask.h"
#include "choose.h"
#include "llinesi.h"
#include "errors.h"
#include "liflines.h"
#include "gedcom.h"
#include "messages.h"
#include "codesets.h"
#include "remove.h"
#include "de-strings.h"
#include "ll-sequence.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase

/*=====================================================
 * chooseAndRemoveFamily -- Choose & delete a family
 *  (remove all members, and delete F record)
 *===================================================*/
void
chooseAndRemoveFamily (void)
{
	GNode *fam, *node, *indi;
	Sequence *spseq, *chseq;
	String tag, key;
	char confirm[512]="", members[64];
	char spouses[32], children[32];
	int n;

	fam = nztop(ask_for_fam_by_key(_(qSidfrmv), _(qSidfrsp), _(qSidfrch)));
	if (!fam)
		return;

	/* get list of spouses & children */
	spseq = createSequence(currentDatabase->recordIndex); /* spouses */
	chseq = createSequence(currentDatabase->recordIndex); /* children */
	for (node=nchild(fam); node; node = nsibling(node)) {
		tag = ntag(node);
		if (eqstr("HUSB", tag) || eqstr("WIFE", tag)) {
			key = strsave(nval(node));
#if defined(DEADENDS)
			appendToSequence(spseq, key, NULL);
#else
			append_indiseq_null(spseq, key, NULL, true, true);
#endif
		} else if (eqstr("CHIL", tag)) {
			key = strsave(nval(node));
#if defined(DEADENDS)
			appendToSequence(chseq, key, NULL);
#else
			append_indiseq_null(chseq, key, NULL, true, true);
#endif
		}
	}

	/* build confirm string */
	n = lengthSequence(spseq);
	destrsetf(spouses, sizeof(spouses), uu8
		, _pl(FMT_INT " spouse", FMT_INT " spouses", n), n);
	n = lengthSequence(chseq);
	destrsetf(children, sizeof(children), uu8
		, _pl(FMT_INT " child", FMT_INT " children", n), n);
	destrsetf(members, sizeof(members), uu8
		, _(qScffdeld), familyToKey(fam), spouses, children);
	destrapps(confirm, sizeof(confirm), uu8, _(qScffdel));
	destrapps(confirm, sizeof(confirm), uu8, members);

	if (ask_yes_or_no(confirm)) {

		if (lengthSequence(spseq)+lengthSequence(chseq) == 0) {
			/* handle empty family */
			remove_empty_fam(fam, currentDatabase);
		}
		else {
			/* the last remove command will delete the family */
			FORSEQUENCE(spseq, el, num)
				indi = keyToPerson(element_skey(el), currentDatabase->recordIndex);
				remove_spouse(indi, fam, currentDatabase);
			ENDSEQUENCE

			FORSEQUENCE(chseq, el, num)
				indi = keyToPerson(element_skey(el), currentDatabase->recordIndex);
				remove_child(indi, fam, currentDatabase);
			ENDSEQUENCE
		}
	}
	
	deleteSequence(spseq);
	deleteSequence(chseq);
}
/*================================================================
 * chooseAndRemovePerson -- Prompt & delete person and links; if this leaves families
 *   with no links, remove them
 *  indi:  [in]  person to remove - (if null, will ask for person)
 *  conf:  [in]  have user confirm ?
 *==============================================================*/
void
chooseAndRemovePerson (GNode *indi, CONFIRMQ confirmq)
{
	/* prompt if needed */
	if (!indi && !(indi = nztop(ask_for_indi(_(qSidpdel), DOASK1))))
		return;
	/* confirm if caller desired */
	if (confirmq==DOCONFIRM && !ask_yes_or_no(_(qScfpdel))) return;

	/* alright, we finished the UI, so delegate to the internal workhorse */
	remove_indi_by_root(indi, currentDatabase);
}
/*================================================================
 * chooseAndRemoveAnyRecord -- Prompt & delete any record
 *   (delete any empty families produced)
 *  record:  [in]  record to remove (if null, will ask for record)
 *  conf:  [in]  have user confirm ?
 *==============================================================*/
bool
chooseAndRemoveAnyRecord (GNode *record, CONFIRMQ confirmq)
{
	/* prompt if needed */
	if (!record && !(record = ask_for_any(_(qSidodel), DOASK1)))
		return false;
	/* confirm if caller desired */
	if (confirmq==DOCONFIRM && !ask_yes_or_no(_(qScfodel)))
		return false;

	/* alright, we finished the UI, so delegate to the internal workhorse */
	return remove_any_record(record, currentDatabase);
}
/*===========================================
 * chooseAndRemoveSpouse -- Remove spouse 
 *  from family (prompting for spouse and/or family
 *  if NULL passed)
 *  nolast: don't remove last member of family?
 *=========================================*/
bool
chooseAndRemoveSpouse (GNode *irec, GNode *frec, bool nolast)
{
	GNode *fam;

/* Identify spouse to remove */
	if (!irec) irec = ask_for_indi(_(qSidsrmv), NOASK1);
	if (!irec) return false;
	if (!FAMS(nztop(irec))) {
		msg_error("%s", _(qSntprnt));
		return false;
	}

/* Identify family to remove spouse from */
	if (!frec) frec = chooseFamily(irec, _(qSparadox), _(qSidsrmf), true);
	if (!frec) return false;
	fam = nztop(frec);
	if (nolast && num_fam_xrefs(fam) < 2) {
		msg_error("%s", _(qSnormls));
		return false;
	}
	if (!ask_yes_or_no(_(qScfsrmv))) return false;

	/* call internal workhorse remove_spouse() to do the actual removal */
	if (!remove_spouse(nztop(irec), fam, currentDatabase)) {
		msg_error("%s", _(qSntsinf));
		return false;
	}
	msg_info("%s", _(qSoksrmv));
	return true;
}
/*===========================================
 * chooseAndRemoveChild -- Remove child
 *  from family (prompting for child and/or family
 *  if NULL passed)
 *  nolast: don't remove last member of family?
 *=========================================*/
bool
chooseAndRemoveChild (GNode *irec, GNode *frec, bool nolast)
{
	GNode *fam;

/* Identify child and check for FAMC nodes */
	if (!irec) irec = ask_for_indi(_(qSidcrmv), NOASK1);
	if (!irec) return false;
	if (!FAMC(nztop(irec))) {
		msg_error("%s", _(qSntchld));
		return false;
	}

/* Identify family to remove child from */
	if (!frec) frec = chooseFamily(irec, _(qSparadox), _(qSidcrmf), false);
	if (!frec) return false;
	fam = nztop(frec);
	if (nolast && num_fam_xrefs(fam) < 2) {
		msg_error("%s", _(qSnormls));
		return false;
	}
	if (!ask_yes_or_no(_(qScfcrmv))) return true;

	if (!remove_child(nztop(irec), fam, currentDatabase)) {
		msg_error("%s", _(qSntcinf));
		return false;
	}

	msg_info("%s", _(qSokcrmv));
	return true;
}

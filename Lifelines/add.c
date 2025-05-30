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
 * add.c -- Add new person or family to database; add child to
 *   family; add spouse to family
 * Copyright(c) 1992-95 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   2.3.6 - 29 Oct 93    3.0.0 - 23 Sep 94
 *   3.0.2 - 12 Dec 94    3.0.3 - 20 Jan 96
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#endif

#include "porting.h"
#include "ll-porting.h"
#include "ll-standard.h"
#include "standard.h"
#include "denls.h"
#include "sys_inc.h"
#include "refnindex.h"
#include "gnode.h"
#include "database.h"
#include "ll-addoperations.h"
#include "rfmt.h"
#include "refns.h"
#include "zstr.h"
#include "translat.h"
#include "xlat.h"
#include "readwrite.h"
#include "stringtable.h"
#include "options.h"
#include "gstrings.h"
#include "llpy-externs.h"

#include "recordindex.h"
#include "editing.h"
#include "sequence.h"
#include "errors.h"
#include "ask.h"
#include "feedback.h"
#include "llinesi.h"
#include "liflines.h"
#include "messages.h"
#include "splitjoin.h"
#include "codesets.h"
#include "lineage.h"
#include "xref.h"
#include "de-strings.h"
#include "ui.h"
#include "locales.h"
#include "lloptions.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase

//extern bool traditional;

/*********************************************
 * local function prototypes
 *********************************************/


/*==========================================================
 * get_unresolved_ref_error -- get string for unresolved reference(s)
 *  xgettext doesn't support special keywords for plural forms, AFAIK
 *  so we can't put these in static variables
 *========================================================*/
CString
get_unresolved_ref_error_string (int count)
{
	return _pl("There was %d unresolved reference."
		, "There were %d unresolved references.", count);
}
/*==========================================================
 * add_indi_by_edit -- Add new person to database by editing
 * (with user interaction)
 * returns addref'd record
 *========================================================*/
GNode *
add_indi_by_edit (bool rfmt)
{
	FILE *fp;
	GNode *indi0=0;
	GNode *indi=0;
	String str, msg;
	bool emp;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);

/* Create person template for user to edit */

	if (!(fp = fopen(editfile, DEWRITETEXT)))
		return NULL;
	prefix_file_for_edit(fp);

	/* prefer useroption in this db */
	if ((str = getdeoptstr("INDIREC", NULL)))
		fprintf(fp, "%s\n", str);
	else { /* default */
		fprintf(fp, "0 INDI\n1 NAME Fname/Surname\n1 SEX MF\n");
		fprintf(fp, "1 BIRT\n  2 DATE\n  2 PLAC\n");
		fprintf(fp, "1 DEAT\n  2 DATE\n  2 PLAC\n1 SOUR\n");
	}

/* Have user edit new person record */

	fclose(fp);
	do_edit();
	while (true) {
		int cnt;
		if (indi0) {
			releaseRecord(indi0);
			indi0=0;
		}
		indi0 = file_to_node(editfile, ttmi, &msg, &emp);
		if (!indi0) {
			if (ask_yes_or_no_msg(msg, _(qSiredit))) {
				do_edit();
				continue;
			} 
			break;
		}
		indi = nztop(indi0);
		cnt = resolveRefnLinks(indi, currentDatabase);
		/* check validation & allow user to reedit if invalid */
		/* this is a showstopper, so alternative is to abort */
		if (!valid_indi_tree(indi, &msg, NULL, currentDatabase)) {
			if (ask_yes_or_no_msg(msg, _(qSiredit))) {
				do_edit();
				continue;
			}
			releaseRecord(indi0);
			indi0 = NULL;
			break;
		}
		/* Allow user to reedit if desired if any refn links unresolved */
		/* this is not a showstopper, so alternative is to continue */
		if (cnt > 0) {
			char msgb[120];
			destrncpyf(msgb, sizeof(msgb), uu8
				, get_unresolved_ref_error_string(cnt), cnt);
			if (ask_yes_or_no_msg(msgb, _(qSireditopt))) {
				write_indi_to_file_for_edit(indi, editfile, rfmt, currentDatabase);
				do_edit();
				continue;
			}
		}
		break;
	}
	if (!indi0 || !ask_yes_or_no(_(qScfpadd))) {
		if (indi0) releaseRecord(indi0);
		return NULL;
	}
	
	/* add the new record to the database */
	add_new_indi_to_db(indi0);

	msg_status(_(qSgdpadd), personToName(nztop(indi0), 35));
	return indi0;
}
/*==========================================================
 * add_new_indi_to_db -- Add newly created person to database
 * (no user interaction)
 * creates record & adds to cache
 *========================================================*/
void
add_new_indi_to_db (GNode *indi0)
{
	GNode *name, *refn, *sex, *body, *dumb, *node;
	CString key;
	GNode *indi = nztop(indi0);

	splitPerson(indi, &name, &refn, &sex, &body, &dumb, &dumb);
	key = getNewPersonKey (currentDatabase);
	indi0->key = strdup(key);
	for (node = name; node; node = nsibling(node)) {
		insertInNameIndex (currentDatabase->nameIndex, nval(node), key);
	}
	for (node = refn; node; node = nsibling(node)) {
		if (nval(node))
			addRefn(nval(node), key, database);
	}
	joinPerson(indi, name, refn, sex, body, NULL, NULL);
	resolveRefnLinks(indi, currentDatabase);
	indi_to_dbase(indi);
}
/*================================================================
 * add_indi_no_cache -- Add new person to database
 *  does not insert into cache
 *  (used by import)
 * (no user interaction)
 *==============================================================*/
bool
add_indi_no_cache (GNode *indi)
{
	GNode *node, *name, *refn, *sex, *body, *famc, *fams;
	CString key;
	// Save INDI key value since rmvat static array entries may get reused
	// before we write the INDI out (for example, >32 ASSO tags). This
	// prevents us from writing the record out using the wrong key.
	key = strsave(nxref(indi));

	splitPerson(indi, &name, &refn, &sex, &body, &famc, &fams);
	for (node = name; node; node = nsibling(node))
		insertInNameIndex (currentDatabase->nameIndex, nval(node), key);
	for (node = refn; node; node = nsibling(node))
		if (nval(node)) addRefn(nval(node), key, database);
	joinPerson(indi, name, refn, sex, body, famc, fams);
	resolveRefnLinks(indi, currentDatabase);
	return storeRecord (database, indi);
}
/*========================================================
 * ask_child_order --  ask user in what order to put child
 * (with user interaction)
 *  fam:     [in] children of this family
 *  promptq: [in] what question to ask
 *  rfmt:    [in] reformatting info
 *======================================================*/
int
ask_child_order (GNode *fam, PROMPTQ promptq, bool rfmt)
{
	int i, nchildren;
	String *childstrings, *childkeys;
/* If first child in family, confirm and add */

	childstrings = get_child_strings(fam, rfmt, &nchildren, &childkeys,
					 currentDatabase);
	if (nchildren == 0) {
		if (promptq == ALWAYS_PROMPT && !ask_yes_or_no(_(qScfcadd)))
				return -1;
		i=0;
/* If not first, find where child belongs */
	} else {
		childstrings[nchildren] = _(qSmklast);
		i = chooseFromArray(_(qSidcfam), nchildren+1, childstrings);
	}
	return i;
}
/*==================================
 * prompt_add_child --  Add child to family
 * (with user interaction)
 *  child: [in] new child to add
 *  fam:   [in] family to which to add
 *================================*/
GNode *
prompt_add_child (GNode *child, GNode *fam, bool rfmt)
{
	int i;

/* Identify child if caller did not */

	if (!child) {
		GNode *rec = ask_for_indi(_(qSidchld), DOASK1, currentDatabase);
		child = nztop(rec);
		releaseRecord(rec);
	}
	if (!child) return NULL;

/* Warn if child to add is already in some family */
	if (FAMC(child)) {
		if (!ask_yes_or_no(_(qSiscinf)))
			return NULL;
	}

/* Identify family if caller did not */

	if (!fam) {
		GNode *rec = ask_for_fam(_(qSidprnt), _(qSidsbln), database);
		fam = nztop(rec);
		releaseRecord(rec);
	}
	if (!fam) return NULL;

	i = ask_child_order(fam, ALWAYS_PROMPT, rfmt);
	if (i == -1) return NULL;

/* Add FAMC node to child */

	add_child_to_fam(child, fam, i);
	msg_status(_(qSgdcadd), personToName(child, 35));
	return fam;
}

/*========================================
 * add_child_to_fam -- Add child to family
 * (no user interaction)
 *======================================*/
void
add_child_to_fam (GNode *child, GNode *fam, int i)
{
	GNode *node, *new, *name, *sex, *body, *famc, *fams;
	GNode *husb, *wife, *chil, *rest, *refn, *fref;
	GNode *nfmc, *this, *prev;
	int j;

	splitFamily(fam, &fref, &husb, &wife, &chil, &rest);
	prev = NULL;
	node = chil;
	j = 0;
	if (i == -1) { /* add last */
		if (node) {
			while (node) {
				prev = node;
				node = nsibling(node);
			}
		}
	}
	else {
		while (j++ < i) {
			prev = node;
			node = nsibling(node);
		}
	}
	new = createGNode(NULL, "CHIL", nxref(child), fam);
	nsibling(new) = node;
	if (prev)
		nsibling(prev) = new;
	else
		chil = new;
	joinFamily(fam, fref, husb, wife, chil, rest);

/* Add FAMC node to child */

	splitPerson(child, &name, &refn, &sex, &body, &famc, &fams);
	nfmc = createGNode(NULL, "FAMC", nxref(fam), child);
	prev = NULL;
	this = famc;
	while (this) {
		prev = this;
		this = nsibling(this);
	}
	if (!prev)
		famc = nfmc;
	else
		nsibling(prev) = nfmc;
	joinPerson(child, name, refn, sex, body, famc, fams);

/* Write updated records to database */

	resolveRefnLinks(child, currentDatabase);
	resolveRefnLinks(fam, currentDatabase);
	fam_to_dbase(fam);
	indi_to_dbase(child);
}
/*===================================
 * prompt_add_spouse -- Add spouse to family
 * prompt for family & confirm if needed
 *  spouse:  [IN]  spouse add (optional arg)
 *  fam:     [IN]  family to which to add (optional arg)
 *  conf:    [IN]  whether or not caller wants user to confirm
 * (with user interaction)
 *=================================*/
bool
prompt_add_spouse (GNode *sprec, GNode *frec, bool conf)
{
	SexType sex;
	GNode *spouse, *fam = nztop(frec);

	/* Identify spouse to add to family */

	if (!sprec) sprec = ask_for_indi(_(qSidsadd), DOASK1, currentDatabase);
	if (!sprec) return false;
	spouse = nztop(sprec);
	if ((sex = SEXV(spouse)) == sexUnknown) {
		msg_error("%s", _(qSnosex));
		return false;
	}
/* Identify family to add spouse to */

	if (!fam) fam = nztop(ask_for_fam(_(qSidsinf), _(qSkchild), database));
	if (!fam) return false;

/* Check that spouse can be added */

	if (traditional) {
		GNode *husb, *wife, *chil, *rest, *fref;
		splitFamily(fam, &fref, &husb, &wife, &chil, &rest);
		joinFamily(fam, fref, husb, wife, chil, rest);
		if (sex == sexMale && husb) {
			msg_error("%s", _(qShashsb));
			return false;
		}
		if (sex == sexFemale && wife) {
			msg_error("%s", _(qShaswif));
			return false;
		}
	}

	if (conf && !ask_yes_or_no(_(qScfsadd)))
		return false;

	add_spouse_to_fam(spouse, fam, sex);
	msg_status(_(qSgdsadd), personToName(spouse, 35));
	return true;
}
/*===================================
 * add_spouse_to_fam -- Add spouse to family
 * after all user input
 * (no user interaction)
 *=================================*/
void
add_spouse_to_fam (GNode *spouse, GNode *fam, SexType sex)
{
/* Add HUSB or WIFE node to family */
	GNode *husb, *wife, *chil, *rest, *fams, *prev, *fref, *this, *new;
	splitFamily(fam, &fref, &husb, &wife, &chil, &rest);
	if (sex == sexMale)
	{
		prev = NULL;
		this = husb;
		while (this) {
			prev = this;
			this = nsibling(this);
		}
		new = createGNode(NULL, "HUSB", nxref(spouse), fam);
		if (prev)
			nsibling(prev) = new;
		else
			husb = new;
	} else {
		prev = NULL;
		this = wife;
		while (this) {
			prev = this;
			this = nsibling(this);
		}
		new = createGNode(NULL, "WIFE", nxref(spouse), fam);
		if (prev)
			nsibling(prev) = new;
		else
			wife = new;
	}
	joinFamily(fam, fref, husb, wife, chil, rest);

	/* Add FAMS node to spouse */

	fams = createGNode(NULL, "FAMS", nxref(fam), spouse);
	prev = NULL;
	this = nchild(spouse);
	while (this) {
		prev = this;
		this = nsibling(this);
	}
	ASSERT(prev);
	nsibling(prev) = fams;

	/* Write updated records to database */

	resolveRefnLinks(spouse, currentDatabase);
	resolveRefnLinks(fam, currentDatabase);
	indi_to_dbase(spouse);
	fam_to_dbase(fam);
}
/*=========================================
 * add_members_to_family -- Add members to new family
 * (no user interaction)
 *=======================================*/
static void
add_members_to_family (CString xref, GNode *spouse1, GNode *spouse2, GNode *child)
{
	GNode *refn, *body;
	GNode *name, *sex, *famc, *fams, *node, *prev, *new, *this;
	if (spouse1) {
		new = createGNode(NULL, "FAMS", xref, spouse1);
		prev = NULL;
		node = nchild(spouse1);
		while (node) {
			prev = node;
			node = nsibling(node);
		}
		if (prev)
			nsibling(prev) = new;
		else
			nchild(spouse1) = new;
	}
	if (spouse2) {
		new = createGNode(NULL, "FAMS", xref, spouse2);
		prev = NULL;
		node = nchild(spouse2);
		while (node) {
			prev = node;
			node = nsibling(node);
		}
		if (prev)
			nsibling(prev) = new;
		else
			nchild(spouse2) = new;
	}
	if (child) {
		splitPerson(child, &name, &refn, &sex, &body, &famc, &fams);
		new = createGNode(NULL, "FAMC", xref, child);
		prev = NULL;
		this = famc;
		while (this) {
			prev = this;
			this = nsibling(this);
		}
		if (prev)
			nsibling(prev) = new;
		else
			famc = new;
		joinPerson(child, name, refn, sex, body, famc, fams);
	}
}
/*=========================================
 * add_family_by_edit -- Add new family to database
 * (with user interaction)
 *=======================================*/
GNode *
add_family_by_edit (GNode *sprec1, GNode *sprec2,
		    GNode *chrec, bool rfmt)
{
	SexType sex1 = sexUnknown;
	SexType sex2 = sexUnknown;
	GNode *spouse1, *spouse2, *child;
	GNode *fam1, *fam2=0, *husb, *wife, *chil;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);
	XLAT ttmo = transl_get_predefined_xlat(MINED);
	String msg=0, key=0, str=0;
	bool emp;
	FILE *fp=NULL;

/* Handle case where child is known */

	if (chrec)  {
		sprec1 = sprec2 = NULL;
		goto editfam;
	}

/* Identify first spouse */

	if (!sprec1) 
		sprec1 = ask_for_indi(_(qSidsps1), NOASK1, currentDatabase);
	if (!sprec1) 
		return NULL;
	if ((sex1 = SEXV(nztop(sprec1))) == sexUnknown)
	{
		msg_error("%s", _(qSunksex));
		return NULL;
	}

	/* Identify optional spouse */

	if (!sprec2)
		sprec2 = ask_for_indi(_(qSidsps2), DOASK1, currentDatabase);
	if (sprec2) {
		if ((sex2 = SEXV(nztop(sprec2))) == sexUnknown || 
			(traditional && sex1 == sex2))
		{
			msg_error("%s", _(qSnotopp));
			return NULL;
		}
	}

/* Create new family */
editfam:
	spouse1 = nztop(sprec1);
	spouse2 = nztop(sprec2);
	child = nztop(chrec);

	fam1 = createGNode(NULL, "FAM", NULL, NULL);
	husb = wife = chil = NULL;
	/* support non-traditional families if traditional is false
	 * to do this we make slightly fib about the use of the
	 * terms husb and wife in setting up spouse nodes
	 */
	if (sex1 == sex2 && sex2 == sexFemale) {
	    husb = createGNode(NULL, "WIFE", nxref(spouse1), fam1);
	} else if (sex1 == sexMale ) {
	    husb = createGNode(NULL, "HUSB", nxref(spouse1), fam1);
	} else if (sex2 == sexMale && sex1 != sexMale) {
	    husb = createGNode(NULL, "HUSB", nxref(spouse2), fam1);
	}
	if (sex1 == sexFemale && sex2 != sexFemale) {
	    wife = createGNode(NULL, "WIFE", nxref(spouse1), fam1);
	} else if (sex2 == sexFemale) {
	    wife = createGNode(NULL, "WIFE", nxref(spouse2), fam1);
	} else if (sex1 == sex2 && sex2 == sexMale) {
	    wife = createGNode(NULL, "HUSB", nxref(spouse2), fam1);
	}
	if (child)
		chil = createGNode(NULL, "CHIL", nxref(child), fam1);

/* Prepare file for user to edit */

	ASSERT(fp = fopen(editfile, DEWRITETEXT));
	prefix_file_for_edit(fp);

	write_nodes(0, fp, ttmo, fam1, true, true, true);
	write_nodes(1, fp, ttmo, husb, true, true, true);
	write_nodes(1, fp, ttmo, wife, true, true, true);
	/* prefer user option in db */
	if ((str = getdeoptstr("FAMRECBODY", NULL)))
		fprintf(fp, "%s\n", str);
	else /* default */
		fprintf(fp, "1 MARR\n  2 DATE\n  2 PLAC\n  2 SOUR\n");
	write_nodes(1, fp, ttmo, chil, true, true, true);
	fclose(fp);
	joinFamily(fam1, NULL, husb, wife, chil, NULL);

/* Have user edit family info */

	do_edit();
	while (true) {
		int cnt;
		fam2 = file_to_node(editfile, ttmi, &msg, &emp);
		if (!fam2) {
			if (ask_yes_or_no_msg(msg, _(qSfredit))) {
				do_edit();
				continue;
			}
			break;
		}
		cnt = resolveRefnLinks(fam2, currentDatabase);
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
			destrncpyf(msgb, sizeof(msgb), uu8
				, get_unresolved_ref_error_string(cnt), cnt);
			if (ask_yes_or_no_msg(msgb, _(qSfreditopt))) {
				write_fam_to_file_for_edit(fam2, editfile, rfmt, currentDatabase);
				do_edit();
				continue;
			}
		}
		break;
	}

/* Confirm family add operation */

	freeGNodes(fam1);
	if (!fam2 || !ask_yes_or_no(_(qScffadd))) {
		freeGNodes(fam2);
		return NULL;
	}

	/* Add the new record to the database */
	add_new_fam_to_db(fam2, spouse1, spouse2, child);

	msg_info("%s", _(qSgdfadd));

	key = nxref(fam2);
	return keyToFamilyRecord(key, currentDatabase);
}
/*==========================================================
 * add_new_fam_to_db -- Add newly created family to database
 * (no user interaction)
 * creates record & adds to cache
 *========================================================*/
void
add_new_fam_to_db (GNode *fam2, GNode *spouse1, GNode *spouse2, GNode *child)
{
	GNode *refn, *husb, *wife, *chil, *body;
	GNode *node;
	String key=0;
	CString xref = getNewFamilyKey(currentDatabase);

	nxref(fam2) = strsave(xref);

/* Modify spouse/s and/or child */

	add_members_to_family(xref, spouse1, spouse2, child);

/* Write updated records to database */

	splitFamily(fam2, &refn, &husb, &wife, &chil, &body);
	key = nxref(fam2);
	for (node = refn; node; node = nsibling(node))
		if (nval(node)) addRefn(nval(node), key, database);
	joinFamily(fam2, refn, husb, wife, chil, body);
	resolveRefnLinks(fam2, currentDatabase);
	resolveRefnLinks(spouse1, currentDatabase);
	resolveRefnLinks(spouse2, currentDatabase);
	resolveRefnLinks(child, currentDatabase);
	fam_to_dbase(fam2);
	if (spouse1) indi_to_dbase(spouse1);
	if (spouse2) indi_to_dbase(spouse2);
	if (child) indi_to_dbase(child);
}
#ifdef ETHEL
/*=========================================
 * add_family_to_db -- Add new family to database
 * (no user interaction)
 * This is stolen from add_family
 * and may not be terribly efficient - Perry
 *=======================================*/
GNode *
add_family_to_db (GNode *spouse1, GNode *spouse2, GNode *child)
{
	SexType sex1 = spouse1 ? SEXV(spouse1) : sexUnknown;
	SexType sex2 = spouse1 ? SEXV(spouse2) : sexUnknown;
	GNode *fam1, *fam2, *refn, *husb, *wife, *chil, *body;
	GNode *node;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);
	XLAT ttmo = transl_get_predefined_xlat(MINED);
	String xref, msg, key;
	bool emp;
	FILE *fp;

	fam1 = createGNode(NULL, "FAM", NULL, NULL);
	husb = wife = chil = NULL;
	if (spouse1) {
		if (sex1 == sexMale)
			husb = createGNode(NULL, "HUSB", nxref(spouse1), fam1);
		else
			wife = createGNode(NULL, "WIFE", nxref(spouse1), fam1);
	}
	if (spouse2) {
		if (sex2 == sexMale)
			husb = createGNode(NULL, "HUSB", nxref(spouse2), fam1);
		else
			wife = createGNode(NULL, "WIFE", nxref(spouse2), fam1);
	}
	if (child)
		chil = createGNode(NULL, "CHIL", nxref(child), fam1);

/* Create file */

	ASSERT(fp = fopen(editfile, DEWRITETEXT));
	write_nodes(0, fp, tto, fam1, true, true, true);
	write_nodes(1, fp, tto, husb, true, true, true);
	write_nodes(1, fp, tto, wife, true, true, true);
	fprintf(fp, "1 MARR\n  2 DATE\n  2 PLAC\n  2 SOUR\n");
	write_nodes(1, fp, tto, chil, true, true, true);
	fclose(fp);
	joinFamily(fam1, NULL, husb, wife, chil, NULL);

	fam2 = file_to_node(editfile, tti, &msg, &emp);

	freeGNodes(fam1);

	add_new_fam_to_db(fam2, spouse1, spouse2, child);

	return fam2;
}
#endif /* ETHEL */

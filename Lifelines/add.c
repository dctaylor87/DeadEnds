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

#if defined(DEADENDS)
#include <ansidecl.h>
#include <stdarg.h>

#if defined(HAVE_INTTYPES_H)
#include <inttypes.h>
#endif

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "llnls.h"
#include "sys_inc.h"
#include "gnode.h"
#include "readwrite.h"
#include "options.h"
#include "llpy-externs.h"

#include "zstr.h"
#include "translat.h"
#include "recordindex.h"
#include "rfmt.h"
#include "editing.h"
#include "sequence.h"
#include "xlat.h"
#include "uiprompts.h"
#include "feedback.h"
#include "llinesi.h"
#include "errors.h"
#include "liflines.h"
#include "messages.h"
#include "splitjoin.h"
#include "codesets.h"
#include "readwrite.h"
#include "lineage.h"
#include "xreffile.h"
#include "de-strings.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase

#else

#include "llstdlib.h"
#include "table.h"
#include "translat.h"
#include "gedcom.h"
#include "indiseq.h"
#include "liflines.h"
#include "llinesi.h"
#include "feedback.h"
#include "lloptions.h"
#include "messages.h"
#endif

extern BOOLEAN traditional;

/*********************************************
 * local function prototypes
 *********************************************/


/*==========================================================
 * get_unresolved_ref_error -- get string for unresolved reference(s)
 *  xgettext doesn't support special keywords for plural forms, AFAIK
 *  so we can't put these in static variables
 *========================================================*/
STRING
get_unresolved_ref_error_string (INT count)
{
	return _pl("There was %d unresolved reference."
		, "There were %d unresolved references.", count);
}
/*==========================================================
 * add_indi_by_edit -- Add new person to database by editing
 * (with user interaction)
 * returns addref'd record
 *========================================================*/
RECORD
add_indi_by_edit (RFMT rfmt)
{
	FILE *fp;
	RECORD indi0=0;
	NODE indi=0;
	STRING str, msg;
	BOOLEAN emp;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);

	if (readonly) {
		msg_error("%s", _(qSronlya));
		return NULL;
	}

/* Create person template for user to edit */

	if (!(fp = fopen(editfile, LLWRITETEXT)))
		return NULL;
	prefix_file_for_edit(fp);

	/* prefer useroption in this db */
	if ((str = getlloptstr("INDIREC", NULL)))
		fprintf(fp, "%s\n", str);
	else { /* default */
		fprintf(fp, "0 INDI\n1 NAME Fname/Surname\n1 SEX MF\n");
		fprintf(fp, "1 BIRT\n  2 DATE\n  2 PLAC\n");
		fprintf(fp, "1 DEAT\n  2 DATE\n  2 PLAC\n1 SOUR\n");
	}

/* Have user edit new person record */

	fclose(fp);
	do_edit();
	while (TRUE) {
		INT cnt;
		if (indi0) {
			release_record(indi0);
			indi0=0;
		}
		indi0 = file_to_record(editfile, ttmi, &msg, &emp);
		if (!indi0) {
			if (ask_yes_or_no_msg(msg, _(qSiredit))) {
				do_edit();
				continue;
			} 
			break;
		}
		indi = nztop(indi0);
		cnt = resolve_refn_links(indi);
		/* check validation & allow user to reedit if invalid */
		/* this is a showstopper, so alternative is to abort */
		if (!valid_indi_tree(indi, &msg, NULL)) {
			if (ask_yes_or_no_msg(msg, _(qSiredit))) {
				do_edit();
				continue;
			}
			release_record(indi0);
			indi0 = NULL;
			break;
		}
		/* Allow user to reedit if desired if any refn links unresolved */
		/* this is not a showstopper, so alternative is to continue */
		if (cnt > 0) {
			char msgb[120];
			llstrncpyf(msgb, sizeof(msgb), uu8
				, get_unresolved_ref_error_string(cnt), cnt);
			if (ask_yes_or_no_msg(msgb, _(qSireditopt))) {
				write_indi_to_file_for_edit(indi, editfile, rfmt);
				do_edit();
				continue;
			}
		}
		break;
	}
	if (!indi0 || !ask_yes_or_no(_(qScfpadd))) {
		if (indi0) release_record(indi0);
		return NULL;
	}
	
	/* add the new record to the database */
	add_new_indi_to_db(indi0);

	msg_status(_(qSgdpadd), indi_to_name(nztop(indi0), 35));
	return indi0;
}
/*==========================================================
 * add_new_indi_to_db -- Add newly created person to database
 * (no user interaction)
 * creates record & adds to cache
 *========================================================*/
void
add_new_indi_to_db (RECORD indi0)
{
#if defined(DEADENDS)
	GNode *name, *refn, *sex, *body, *dumb, *node;
#else
	NODE name, refn, sex, body, dumb, node;
#endif
	char key[MAXKEYWIDTH]="";
	INT32 keynum=0;
	NODE indi = nztop(indi0);

	split_indi_old(indi, &name, &refn, &sex, &body, &dumb, &dumb);
	keynum = getixrefnum();
	snprintf(key, sizeof(key), "I" FMT_INT32, keynum);
	init_new_record(indi0, key);
	for (node = name; node; node = nsibling(node)) {
		add_name(nval(node), key);
	}
	for (node = refn; node; node = nsibling(node)) {
		if (nval(node))
			add_refn(nval(node), key);
	}
	join_indi(indi, name, refn, sex, body, NULL, NULL);
	resolve_refn_links(indi);
	indi_to_dbase(indi);
#if !defined(DEADENDS)
	add_new_indi_to_cache(indi0);
#endif
}
/*================================================================
 * add_indi_no_cache -- Add new person to database
 *  does not insert into cache
 *  (used by import)
 * (no user interaction)
 *==============================================================*/
BOOLEAN
add_indi_no_cache (NODE indi)
{
#if defined(DEADENDS)
	GNode *node, *name, *refn, *sex, *body, *famc, *fams;
#else
	NODE node, name, refn, sex, body, famc, fams;
#endif
	STRING str, key;

	// Save INDI key value since rmvat static array entries may get reused
	// before we write the INDI out (for example, >32 ASSO tags). This
	// prevents us from writing the record out using the wrong key.
	key = strsave(rmvat(nxref(indi)));

	split_indi_old(indi, &name, &refn, &sex, &body, &famc, &fams);
	for (node = name; node; node = nsibling(node))
		add_name(nval(node), key);
	for (node = refn; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);
	join_indi(indi, name, refn, sex, body, famc, fams);
	resolve_refn_links(indi);
	str = node_to_string(indi);
	store_record(key, str, strlen(str));
	stdfree(str);
	stdfree(key);
	return TRUE;
}
/*========================================================
 * ask_child_order --  ask user in what order to put child
 * (with user interaction)
 *  fam:     [in] children of this family
 *  promptq: [in] what question to ask
 *  rfmt:    [in] reformatting info
 *======================================================*/
INT
ask_child_order (NODE fam, PROMPTQ promptq, RFMT rfmt)
{
	INT i, nchildren;
	STRING *childstrings, *childkeys;
/* If first child in family, confirm and add */

	childstrings = get_child_strings(fam, rfmt, &nchildren, &childkeys);
	if (nchildren == 0) {
		if (promptq == ALWAYS_PROMPT && !ask_yes_or_no(_(qScfcadd)))
				return -1;
		i=0;
/* If not first, find where child belongs */
	} else {
		childstrings[nchildren] = _(qSmklast);
		i = choose_from_array(_(qSidcfam), nchildren+1, childstrings);
	}
	return i;
}
/*==================================
 * prompt_add_child --  Add child to family
 * (with user interaction)
 *  child: [in] new child to add
 *  fam:   [in] family to which to add
 *================================*/
NODE
prompt_add_child (NODE child, NODE fam, RFMT rfmt)
{
	INT i;

	if (readonly) {
		msg_error("%s", _(qSronlye));
		return NULL;
	}

/* Identify child if caller did not */

	if (!child) {
		RECORD rec = ask_for_indi(_(qSidchld), DOASK1);
		child = nztop(rec);
		release_record(rec);
	}
	if (!child) return NULL;

/* Warn if child to add is already in some family */
	if (FAMC(child)) {
		if (!ask_yes_or_no(_(qSiscinf)))
			return NULL;
	}

/* Identify family if caller did not */

	if (!fam) {
		RECORD rec = ask_for_fam(_(qSidprnt), _(qSidsbln));
		fam = nztop(rec);
		release_record(rec);
	}
	if (!fam) return NULL;

	i = ask_child_order(fam, ALWAYS_PROMPT, rfmt);
	if (i == -1) return NULL;

/* Add FAMC node to child */

	add_child_to_fam(child, fam, i);
	msg_status(_(qSgdcadd), indi_to_name(child, 35));
	return fam;
}

/*========================================
 * add_child_to_fam -- Add child to family
 * (no user interaction)
 *======================================*/
void
add_child_to_fam (NODE child, NODE fam, INT i)
{
#if defined(DEADENDS)
	GNode *node, *new, *name, *sex, *body, *famc, *fams;
	GNode *husb, *wife, *chil, *rest, *refn, *fref;
	GNode *nfmc, *this, *prev;
#else
	NODE node, new, name, sex, body, famc, fams;
	NODE husb, wife, chil, rest, refn, fref;
	NODE nfmc, this, prev;
#endif
	INT j;

	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
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
	new = create_node(NULL, "CHIL", nxref(child), fam);
	nsibling(new) = node;
	if (prev)
		nsibling(prev) = new;
	else
		chil = new;
	join_fam(fam, fref, husb, wife, chil, rest);

/* Add FAMC node to child */

	split_indi_old(child, &name, &refn, &sex, &body, &famc, &fams);
	nfmc = create_node(NULL, "FAMC", nxref(fam), child);
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
	join_indi(child, name, refn, sex, body, famc, fams);

/* Write updated records to database */

	resolve_refn_links(child);
	resolve_refn_links(fam);
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
BOOLEAN
prompt_add_spouse (RECORD sprec, RECORD frec, BOOLEAN conf)
{
#if defined(DEADENDS)
	SexType sex;
	GNode *spouse, *fam = nztop(frec);
#else
	INT sex;
	NODE spouse, fam = nztop(frec);
#endif

	if (readonly) {
		msg_error("%s", _(qSronlye));
		return FALSE;
	}

	/* Identify spouse to add to family */

	if (!sprec) sprec = ask_for_indi(_(qSidsadd), DOASK1);
	if (!sprec) return FALSE;
	spouse = nztop(sprec);
#if defined(DEADENDS)
	if ((sex = SEXV(spouse)) == sexUnknown) {
		msg_error("%s", _(qSnosex));
		return FALSE;
	}
#else
	if ((sex = SEX(spouse)) == SEX_UNKNOWN) {
	  msg_error("%s", _(qSnosex));
	  return FALSE;
	}
#endif
/* Identify family to add spouse to */

	if (!fam) fam = nztop(ask_for_fam(_(qSidsinf), _(qSkchild)));
	if (!fam) return FALSE;

/* Check that spouse can be added */

	if (traditional) {
#if defined(DEADENDS)
		GNode *husb, *wife, *chil, *rest, *fref;
#else
		NODE husb, wife, chil, rest, fref;
#endif
		split_fam(fam, &fref, &husb, &wife, &chil, &rest);
		join_fam(fam, fref, husb, wife, chil, rest);
#if defined(DEADENDS)
		if (sex == sexMale && husb) {
			msg_error("%s", _(qShashsb));
			return FALSE;
		}
		if (sex == sexFemale && wife) {
			msg_error("%s", _(qShaswif));
			return FALSE;
		}
#else
		if (sex == SEX_MALE && husb) {
			msg_error("%s", _(qShashsb));
			return FALSE;
		}
		if (sex == SEX_FEMALE && wife) {
			msg_error("%s", _(qShaswif));
			return FALSE;
		}
#endif
	}

	if (conf && !ask_yes_or_no(_(qScfsadd)))
		return FALSE;

	add_spouse_to_fam(spouse, fam, sex);
	msg_status(_(qSgdsadd), indi_to_name(spouse, 35));
	return TRUE;
}
/*===================================
 * add_spouse_to_fam -- Add spouse to family
 * after all user input
 * (no user interaction)
 *=================================*/
void
#if defined(DEADENDS)
add_spouse_to_fam (NODE spouse, NODE fam, SexType sex)
#else
add_spouse_to_fam (NODE spouse, NODE fam, INT sex)
#endif
{
/* Add HUSB or WIFE node to family */
#if defined(DEADENDS)
	GNode *husb, *wife, *chil, *rest, *fams, *prev, *fref, *this, *new;
#else
	NODE husb, wife, chil, rest, fams, prev, fref, this, new;
#endif
	split_fam(fam, &fref, &husb, &wife, &chil, &rest);
#if defined(DEADENDS)
	if (sex == sexMale)
#else
	if (sex == SEX_MALE)
#endif
	{
		prev = NULL;
		this = husb;
		while (this) {
			prev = this;
			this = nsibling(this);
		}
		new = create_node(NULL, "HUSB", nxref(spouse), fam);
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
		new = create_node(NULL, "WIFE", nxref(spouse), fam);
		if (prev)
			nsibling(prev) = new;
		else
			wife = new;
	}
	join_fam(fam, fref, husb, wife, chil, rest);

	/* Add FAMS node to spouse */

	fams = create_node(NULL, "FAMS", nxref(fam), spouse);
	prev = NULL;
	this = nchild(spouse);
	while (this) {
		prev = this;
		this = nsibling(this);
	}
	ASSERT(prev);
	nsibling(prev) = fams;

	/* Write updated records to database */

	resolve_refn_links(spouse);
	resolve_refn_links(fam);
	indi_to_dbase(spouse);
	fam_to_dbase(fam);
}
/*=========================================
 * add_members_to_family -- Add members to new family
 * (no user interaction)
 *=======================================*/
static void
add_members_to_family (STRING xref, NODE spouse1, NODE spouse2, NODE child)
{
#if defined(DEADENDS)
	GNode *refn, *body;
	GNode *name, *sex, *famc, *fams, *node, *prev, *new, *this;
#else
	NODE refn, body;
	NODE name, sex, famc, fams, node, prev, new, this;
#endif
	if (spouse1) {
		new = create_node(NULL, "FAMS", xref, spouse1);
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
		new = create_node(NULL, "FAMS", xref, spouse2);
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
		split_indi_old(child, &name, &refn, &sex, &body, &famc, &fams);
		new = create_node(NULL, "FAMC", xref, child);
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
		join_indi(child, name, refn, sex, body, famc, fams);
	}
}
/*=========================================
 * add_family_by_edit -- Add new family to database
 * (with user interaction)
 *=======================================*/
RECORD
add_family_by_edit (RECORD sprec1, RECORD sprec2, RECORD chrec, RFMT rfmt)
{
#if defined(DEADENDS)
	SexType sex1 = sexUnknown;
	SexType sex2 = sexUnknown;
	GNode *spouse1, *spouse2, *child;
	GNode *fam1, *fam2=0, *husb, *wife, *chil;
#else
	INT sex1 = 0;
	INT sex2 = 0;
	NODE spouse1, spouse2, child;
	NODE fam1, fam2=0, husb, wife, chil;
#endif
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);
	XLAT ttmo = transl_get_predefined_xlat(MINED);
	STRING msg=0, key=0, str=0;
	BOOLEAN emp;
	FILE *fp=NULL;

	if (readonly) {
		msg_error("%s", _(qSronlya));
		return NULL;
	}

/* Handle case where child is known */

	if (chrec)  {
		sprec1 = sprec2 = NULL;
		goto editfam;
	}

/* Identify first spouse */

	if (!sprec1) 
		sprec1 = ask_for_indi(_(qSidsps1), NOASK1);
	if (!sprec1) 
		return NULL;
#if defined(DEADENDS)
	if ((sex1 = SEXV(nztop(sprec1))) == sexUnknown)
#else
	if ((sex1 = SEX(nztop(sprec1))) == SEX_UNKNOWN)
#endif
	{
		msg_error("%s", _(qSunksex));
		return NULL;
	}

	/* Identify optional spouse */

	if (!sprec2)
		sprec2 = ask_for_indi(_(qSidsps2), DOASK1);
	if (sprec2) {
#if defined(DEADENDS)
		if ((sex2 = SEXV(nztop(sprec2))) == sexUnknown || 
			(traditional && sex1 == sex2))
#else
		if ((sex2 = SEX(nztop(sprec2))) == SEX_UNKNOWN || 
			(traditional && sex1 == sex2))
#endif
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

	fam1 = create_node(NULL, "FAM", NULL, NULL);
	husb = wife = chil = NULL;
	/* support non-traditional families if traditional is false
	 * to do this we make slightly fib about the use of the
	 * terms husb and wife in setting up spouse nodes
	 */
#if defined(DEADENDS)
	if (sex1 == sex2 && sex2 == sexFemale) {
	    husb = create_node(NULL, "WIFE", nxref(spouse1), fam1);
	} else if (sex1 == sexMale ) {
	    husb = create_node(NULL, "HUSB", nxref(spouse1), fam1);
	} else if (sex2 == sexMale && sex1 != sexMale) {
	    husb = create_node(NULL, "HUSB", nxref(spouse2), fam1);
	}
	if (sex1 == sexFemale && sex2 != sexFemale) {
	    wife = create_node(NULL, "WIFE", nxref(spouse1), fam1);
	} else if (sex2 == sexFemale) {
	    wife = create_node(NULL, "WIFE", nxref(spouse2), fam1);
	} else if (sex1 == sex2 && sex2 == sexMale) {
	    wife = create_node(NULL, "HUSB", nxref(spouse2), fam1);
	}
#else
	if (sex1 == sex2 && sex2 == SEX_FEMALE) {
	    husb = create_node(NULL, "WIFE", nxref(spouse1), fam1);
	} else if (sex1 == SEX_MALE ) {
	    husb = create_node(NULL, "HUSB", nxref(spouse1), fam1);
	} else if (sex2 == SEX_MALE && sex1 != SEX_MALE) {
	    husb = create_node(NULL, "HUSB", nxref(spouse2), fam1);
	}
	if (sex1 == SEX_FEMALE && sex2 != SEX_FEMALE) {
	    wife = create_node(NULL, "WIFE", nxref(spouse1), fam1);
	} else if (sex2 == SEX_FEMALE) {
	    wife = create_node(NULL, "WIFE", nxref(spouse2), fam1);
	} else if (sex1 == sex2 && sex2 == SEX_MALE) {
	    wife = create_node(NULL, "HUSB", nxref(spouse2), fam1);
	}
#endif
	if (child)
		chil = create_node(NULL, "CHIL", nxref(child), fam1);

/* Prepare file for user to edit */

	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	prefix_file_for_edit(fp);

	write_nodes(0, fp, ttmo, fam1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, husb, TRUE, TRUE, TRUE);
	write_nodes(1, fp, ttmo, wife, TRUE, TRUE, TRUE);
	/* prefer user option in db */
	if ((str = getlloptstr("FAMRECBODY", NULL)))
		fprintf(fp, "%s\n", str);
	else /* default */
		fprintf(fp, "1 MARR\n  2 DATE\n  2 PLAC\n  2 SOUR\n");
	write_nodes(1, fp, ttmo, chil, TRUE, TRUE, TRUE);
	fclose(fp);
	join_fam(fam1, NULL, husb, wife, chil, NULL);

/* Have user edit family info */

	do_edit();
	while (TRUE) {
		INT cnt;
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
		if (!valid_fam_tree(fam2, &msg, fam1)) {
			if (ask_yes_or_no_msg(msg, _(qSfredit))) {
				do_edit();
				continue;
			}
			free_nodes(fam2);
			fam2 = NULL;
			break;
		}
		/* Allow user to reedit if desired if any refn links unresolved */
		/* this is not a showstopper, so alternative is to continue */
		if (cnt > 0) {
			char msgb[120];
			llstrncpyf(msgb, sizeof(msgb), uu8
				, get_unresolved_ref_error_string(cnt), cnt);
			if (ask_yes_or_no_msg(msgb, _(qSfreditopt))) {
				write_fam_to_file_for_edit(fam2, editfile, rfmt);
				do_edit();
				continue;
			}
		}
		break;
	}

/* Confirm family add operation */

	free_nodes(fam1);
	if (!fam2 || !ask_yes_or_no(_(qScffadd))) {
		free_nodes(fam2);
		return NULL;
	}

	/* Add the new record to the database */
	add_new_fam_to_db(fam2, spouse1, spouse2, child);

	msg_info("%s", _(qSgdfadd));

	key = rmvat(nxref(fam2));
	return key_to_record(key);
}
/*==========================================================
 * add_new_fam_to_db -- Add newly created family to database
 * (no user interaction)
 * creates record & adds to cache
 *========================================================*/
void
add_new_fam_to_db (NODE fam2, NODE spouse1, NODE spouse2, NODE child)
{
#if defined(DEADENDS)
	GNode *refn, *husb, *wife, *chil, *body;
	GNode *node;
#else
	NODE refn, husb, wife, chil, body;
	NODE node;
#endif
	STRING key=0;
	STRING xref = getfxref();

	nxref(fam2) = strsave(xref);

/* Modify spouse/s and/or child */

	add_members_to_family(xref, spouse1, spouse2, child);

/* Write updated records to database */

	split_fam(fam2, &refn, &husb, &wife, &chil, &body);
	key = rmvat(nxref(fam2));
	for (node = refn; node; node = nsibling(node))
		if (nval(node)) add_refn(nval(node), key);
	join_fam(fam2, refn, husb, wife, chil, body);
	resolve_refn_links(fam2);
	resolve_refn_links(spouse1);
	resolve_refn_links(spouse2);
	resolve_refn_links(child);
	fam_to_dbase(fam2);
#if !defined(DEADENDS)
	fam_to_cache(fam2);
#endif
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
NODE
add_family_to_db (NODE spouse1, NODE spouse2, NODE child)
{
#if defined(DEADENDS)
	SexType sex1 = spouse1 ? SEXV(spouse1) : sexUnknown;
	SexType sex2 = spouse1 ? SEXV(spouse2) : sexUnknown;
#else
	INT sex1 = spouse1 ? SEX(spouse1) : SEX_UNKNOWN;
	INT sex2 = spouse1 ? SEX(spouse2) : SEX_UNKNOWN;
#endif
	NODE fam1, fam2, refn, husb, wife, chil, body;
	NODE node;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);
	XLAT ttmo = transl_get_predefined_xlat(MINED);
	STRING xref, msg, key;
	BOOLEAN emp;
	FILE *fp;

	fam1 = create_node(NULL, "FAM", NULL, NULL);
	husb = wife = chil = NULL;
	if (spouse1) {
#if defined(DEADENDS)
		if (sex1 == sexMale)
#else
		if (sex1 == SEX_MALE)
#endif
			husb = create_node(NULL, "HUSB", nxref(spouse1), fam1);
		else
			wife = create_node(NULL, "WIFE", nxref(spouse1), fam1);
	}
	if (spouse2) {
#if defined(DEADENDS)
		if (sex2 == sexMale)
#else
		if (sex2 == SEX_MALE)
#endif
			husb = create_node(NULL, "HUSB", nxref(spouse2), fam1);
		else
			wife = create_node(NULL, "WIFE", nxref(spouse2), fam1);
	}
	if (child)
		chil = create_node(NULL, "CHIL", nxref(child), fam1);

/* Create file */

	ASSERT(fp = fopen(editfile, LLWRITETEXT));
	write_nodes(0, fp, tto, fam1, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, husb, TRUE, TRUE, TRUE);
	write_nodes(1, fp, tto, wife, TRUE, TRUE, TRUE);
	fprintf(fp, "1 MARR\n  2 DATE\n  2 PLAC\n  2 SOUR\n");
	write_nodes(1, fp, tto, chil, TRUE, TRUE, TRUE);
	fclose(fp);
	join_fam(fam1, NULL, husb, wife, chil, NULL);

	fam2 = file_to_node(editfile, tti, &msg, &emp);

	free_nodes(fam1);

	add_new_fam_to_db(fam2, spouse1, spouse2, child);

	return fam2;
}
#endif /* ETHEL */

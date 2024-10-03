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
 * merge.c -- Merge persons and families
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 23 Sep 94    3.0.2 - 13 Dec 94
 *   3.0.3 - 21 Jan 96
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
#include "options.h"

#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "feedback.h"
#include "refnindex.h"
#include "gnode.h"
#include "readwrite.h"
#include "recordindex.h"
#include "rfmt.h"
#include "editing.h"
#include "sequence.h"
#include "hashtable.h"
#include "integertable.h"
#include "xlat.h"
#include "ask.h"
#include "llinesi.h"
#include "errors.h"
#include "feedback.h"
#include "liflines.h"
#include "messages.h"
#include "splitjoin.h"
#include "remove.h"
#include "nodeutils.h"
#include "ll-addoperations.h"
#include "browse.h"
#include "refns.h"
#include "xreffile.h"
#include "name.h"

#include "llpy-externs.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase

#if !defined(NUMBER_LINKAGE_BUCKETS)
#define NUMBER_LINKAGE_BUCKETS	37
#endif
int num_linkage_buckets = NUMBER_LINKAGE_BUCKETS;

/*********************************************
 * external/imported variables
 *********************************************/

extern bool traditional;

static void merge_fam_links(GNode *, GNode *, GNode *, GNode *, int);
static GNode *remove_dupes(GNode *, GNode *);
static GNode *sort_children(GNode *, GNode *);


/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void check_indi_lineage_links(GNode *indi);
static void check_fam_lineage_links(GNode *fam);


/*================================================================
 * merge_two_indis -- Merge first person to second; data from both
 *   are put in file that user edits; first person removed
 *  indi1: [IN]  person who will be deleted (non-null)
 *  indi2: [IN]  person who will receive new (combined & edited) data (non-null)
 *  conf:  [IN]  should we prompt user to confirm change ?
 *---------------------------------------------------------------
 *  These are the four main variables
 *   indi1 - person in database -- indi1 is merged into indi2
 *   indi2 - person in database -- indi1 is merged into this person
 *   indi3 - merged version of the two persons before editing
 *   indi4 - merged version of the two persons after editing
 *==============================================================*/
RecordIndexEl *
merge_two_indis (GNode *indi1, GNode *indi2, bool conf)
{
	GNode *indi01, *indi02;	/* original arguments */
	GNode *name1, *refn1, *sex1, *body1, *famc1, *fams1;
	GNode *name2, *refn2, *sex2, *body2, *famc2, *fams2;
	GNode *indi3, *name3, *refn3, *sex3, *body3, *famc3, *fams3;
	GNode *indi4=0;
	GNode *fam, *husb, *wife, *chil, *rest, *fref, *keep=NULL;
	GNode *this, *that, *prev, *next, *node, *head;
	GNode *fam12;
	GNode *name24, *refn24;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);
	XLAT ttmo = transl_get_predefined_xlat(MINED);
	FILE *fp;
	SexType sx2;
	String msg, key;
 	bool emp;

/* Do start up checks */

	ASSERT(indi1);
	ASSERT(indi2);
	ASSERT(eqstr("INDI", ntag(indi1)));
	ASSERT(eqstr("INDI", ntag(indi2)));
	if (indi1 == indi2) {
		msg_error("%s", _(qSnopmrg));
		return NULL;
	}

/* Check restrictions on persons */

	famc1 = FAMC(indi1);
	famc2 = FAMC(indi2);
/*LOOSEEND -- THIS CHECK IS NOT GOOD ENOUGH */
/* comment from merge.c 1.2 2000-01-03 nozell */
	if (traditional) {
		if (famc1 && famc2 && nestr(nval(famc1), nval(famc2))) {
			if (!ask_yes_or_no_msg(_(qSmgsfam), _(qSmgconf))) {
				msg_error("%s", _(qSnoqmrg));
				return NULL;
			}
		}
	}
	fams1 = FAMS(indi1);
	fams2 = FAMS(indi2);
	if (fams1 && fams2 && SEXV(indi1) != SEXV(indi2)) {
		msg_error("%s", _(qSnoxmrg));
		return NULL;
	}

/* sanity check lineage links */
	check_indi_lineage_links(indi1);
	check_indi_lineage_links(indi2);

/* Split original persons */

	/* If we are successful, the original indi1 will be deleted
 	 * and the indi2 will be updated with the new info.
   	 * However, if we do not merge then we must
	 * insure that indi1 and indi2 are in their original state.
	 * It seems safer to do this by only working with copies
    	 * of the originals.
	 */
	indi01 = indi1;	/* keep original indi1 for later delete */
	indi1 = copyNodes(indi1, true, true);
	indi02 = indi2;	/* keep original indi2 for later update and return */
	indi2 = copyNodes(indi2, true, true);

/* we split indi1 & indi2 and leave them split until near the end */
	splitPerson(indi1, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	splitPerson(indi2, &name2, &refn2, &sex2, &body2, &famc2, &fams2);
	indi3 = indi2; 
	indi2 = copyNodes(indi2, true, true);
	sx2 = sexUnknown;
	if (fams1) sx2 = valueToSex(sex1);
	if (fams2) sx2 = valueToSex(sex2);

/*CONDITION: 1s, 2s - build first version of merged person */

	ASSERT(fp = fopen(editfile, DEWRITETEXT));
	name3 = unionNodes(name1, name2, true, true);
	refn3 = unionNodes(refn1, refn2, true, true);
	sex3  = unionNodes(sex1,  sex2,  true, true);
	body3 = unionNodes(body1, body2, true, true);
	famc3 = unionNodes(famc1, famc2, true, true);
	fams3 = unionNodes(fams1, fams2, true, true);
	write_nodes(0, fp, ttmo, indi3, true, true, true);
	write_nodes(1, fp, ttmo, name3, true, true, true);
	write_nodes(1, fp, ttmo, refn3, true, true, true);
	write_nodes(1, fp, ttmo, sex3,  true, true, true);
	write_nodes(1, fp, ttmo, body3, true, true, true);
	write_nodes(1, fp, ttmo, famc3, true, true, true);
	write_nodes(1, fp, ttmo, fams3, true, true, true);
	fclose(fp);
	joinPerson(indi3, name3, refn3, sex3, body3, famc3, fams3);

/*CONDITION 2 -- 3 (init combined) created and joined*/
/* 
	indi1 & indi2 are originals
	indi3 is combined version
	indi4 will be what user creates editing indi3
	(and then we'll throw away indi3)
*/
/* Have user edit merged person */
	do_edit();
	while (true) {
		indi4 = file_to_node(editfile, ttmi, &msg, &emp);
		if (!indi4 && !emp) {
			if (ask_yes_or_no_msg(msg, _(qSiredit))) {
				do_edit();
				continue;
			} 
			break;
		}
		if (!valid_indi_tree(indi4, &msg, indi3, currentDatabase)) {
			if (ask_yes_or_no_msg(msg, _(qSiredit))) {
				do_edit();
				continue;
			}
			freeGNodes(indi4);
			indi4 = NULL;
			break;
		}
		break;
	}
	freeGNodes(indi3);

/* Have user confirm changes */
	if (!indi4 || (conf && !ask_yes_or_no(_(qScfpmrg)))) {
		if (indi4) freeGNodes(indi4);
		joinPerson(indi1, name1, refn1, sex1, body1, famc1, fams1);
		freeGNodes(indi1);
		joinPerson(indi2, name2, refn2, sex2, body2, famc2, fams2);
		freeGNodes(indi2);
		/* originals (indi01 and indi02) have not been modified */
		return NULL;
	}

/* Modify families that have persons as children */

	classifyNodes(&famc1, &famc2, &fam12);

/*
 process fam12 - the list of FAMC in both original nodes
 Both were children in same family; remove first as child
 FAMCs in indi4 are union of indi1 & indi2 because we don't
 allow the user to edit these 
 */

	this = fam12;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		splitFamily(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		that = chil;
		while (that) {
			if (eqstr(nval(that), nxref(indi1))) {
				next = nsibling(that);
				nsibling(that) = NULL;
				keep = nchild(that);
				free_node(that,"merge_two_indis");
				if (!prev)
					chil = next;
				else
					nsibling(prev) = next;
				that = next;
			} else {
				prev = that;
				that = nsibling(that);
			}
		}
		that = chil;
		while (keep && that) {
			if (eqstr(nval(that), nxref(indi2))) {
				nchild(that) = unionNodes(nchild(that),
					keep, true, false);
			}
			that = nsibling(that);
		}
		joinFamily(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}

/* process famc1 - the list of FAMC only in first original node */
/* Only first was child; make family refer to second */

	this = famc1;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		splitFamily(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		that = chil;
		while (that) {
			if (eqstr(nval(that), nxref(indi1))) {
				stdfree(nval(that));
				nval(that) = strsave(nxref(indi2));
			}
			prev = that;
			that = nsibling(that);
		}
		joinFamily(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}
	freeGNodes(fam12);

/*HERE*/
/* Modify families that had persons as spouse */

	classifyNodes(&fams1, &fams2, &fam12);

/*
 process fam12 - the list of FAMS in both original nodes
 Both were parents in same family; remove first as parent
 FAMSs in indi4 are union of indi1 & indi2 because we don't
 allow the user to edit these 
 */

	this = fam12;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		splitFamily(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		if (sx2 == sexMale)
			head = that = husb;
		else
			head = that = wife;
		while (that) {
			if (eqstr(nval(that), nxref(indi1))) {
				next = nsibling(that);
				nsibling(that) = NULL;
				freeGNodes(that);
				if (!prev)
					prev = head = next;
				else
					nsibling(prev) = next;
				that = next;
			} else {
				prev = that;
				that = nsibling(that);
			}
		}
		if (sx2 == sexMale)
			husb = head;
		else
			wife = head;
		joinFamily(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}

/* process fams1 - the list of FAMS only in first original node */
/* Only first was parent; make family refer to second */

	this = fams1;
	while (this) {
		fam = key_to_fam(rmvat(nval(this)));
		splitFamily(fam, &fref, &husb, &wife, &chil, &rest);
		prev = NULL;
		that = (sx2 == sexMale) ? husb : wife;
		while (that) {
			if (eqstr(nval(that), nxref(indi1))) {
				stdfree(nval(that));
				nval(that) = strsave(nxref(indi2));
			}
			prev = that;
			that = nsibling(that);
		}
		joinFamily(fam, fref, husb, wife, chil, rest);
		fam_to_dbase(fam);
		this = nsibling(this);
	}
	freeGNodes(fam12);

/*
 name1 holds original names of #1/
 name2 holds original names of #2
 indi4 holds new/edited names

 The NAMEs & REFNs in original #1 (indi01) will get deleted
  when indi01 is deleted below
 We just need to take care of any changes from indi02 to indi4
  diff name2 vs name4 & delete any dropped ones, & add new ones
 But instead of messing with indi4, which is the new record
  we'll make a scratch copy (in indi3, which is not used now)
*/

	indi3 = copyNodes(indi4, true, true);

	splitPerson(indi3, &name3, &refn3, &sex3, &body3, &famc3, &fams3);
	classifyNodes(&name2, &name3, &name24);
	classifyNodes(&refn2, &refn3, &refn24);

	key = rmvat(nxref(indi4));
	for (node = name2; node; node = nsibling(node))
		removeFromNameIndex (currentDatabase->nameIndex, nval(node), key); 
	for (node = name3; node; node = nsibling(node))
		insertInNameIndex (currentDatabase->nameIndex, nval(node), key);
	rename_from_browse_lists(key);
	for (node = refn2; node; node = nsibling(node))
		if (nval(node)) remove_refn(nval(node), key);
	for (node = refn3; node; node = nsibling(node))
		if (nval(node)) addRefn(nval(node), key, database);
	joinPerson(indi3, name3, refn3, sex3, body3, famc3, fams3);
	freeGNodes(indi3);
	freeGNodes(name24);
	freeGNodes(refn24);

/* done with changes, save new record to db */

	resolve_refn_links(indi4);
	indi_to_dbase(indi4);

/* finally we're done with indi1 & indi2 */

	joinPerson(indi1, name1, refn1, sex1, body1, famc1, fams1);
	freeGNodes(indi1);
	joinPerson(indi2, name2, refn2, sex2, body2, famc2, fams2);
	freeGNodes(indi2);

/* update indi02 to contain info from new merged record in indi4 */
/* Note - we could probably just save indi4 and delete indi02 
	- Perry, 2000/12/06 */

	splitPerson(indi4, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	splitPerson(indi02, &name2, &refn2, &sex2, &body2, &famc2, &fams2);
	joinPerson(indi4, name2, refn2, sex2, body2, famc2, fams2);
	joinPerson(indi02, name1, refn1, sex1, body1, famc1, fams1);
	freeGNodes(indi4);

	remove_indi_by_root(indi01, currentDatabase);	/* this is the original indi1 */

/* sanity check lineage links */
	check_indi_lineage_links(indi02);

	return node_to_record(indi02);   /* this is the updated indi2 */
}
/*=================================================================
 * merge_two_fams -- Merge first family into second; data from both
 *   are put in file that user edits; first family removed
 *---------------------------------------------------------------
 *  These are the four main variables
 *   fam1 - upper family in database -- fam1 is merged into fam2
 *   fam2 - lower family in database -- fam1 is merged into this person
 *   fam3 - temporary which is the merged version for the user to edit
 *   fam4 - merged version of the two persons after editing
 *     the nodes inside fam4 are stored into fam2 & to dbase at the very end
 *===============================================================*/
RecordIndexEl *
merge_two_fams (GNode *fam1, GNode *fam2)
{
	GNode *husb1, *wife1, *chil1, *rest1, *husb2, *wife2, *chil2, *rest2;
	GNode *fref1, *fref2;
	GNode *fam3, *husb3, *wife3, *chil3, *rest3, *fref3;
	GNode *fam4=0, *husb4, *wife4, *chil4, *rest4, *fref4;
	XLAT ttmi = transl_get_predefined_xlat(MEDIN);
	XLAT ttmo = transl_get_predefined_xlat(MINED);
	FILE *fp;
	String msg;
	bool emp;

	ASSERT(fam1);
	ASSERT(fam2);
	ASSERT(eqstr("FAM", ntag(fam1)));
	ASSERT(eqstr("FAM", ntag(fam2)));
	if (fam1 == fam2) {
		msg_error("%s", _(qSnofmrg));
		return NULL;
	}

/* sanity check lineage links */
	check_fam_lineage_links(fam1);
	check_fam_lineage_links(fam2);

/* Check restrictions on families */
	splitFamily(fam1, &fref1, &husb1, &wife1, &chil1, &rest1);
	splitFamily(fam2, &fref2, &husb2, &wife2, &chil2, &rest2);
	if (traditional) {
		bool ok = true;
		if (husb1 && husb2 && nestr(nval(husb1), nval(husb2))) {
			msg_error("%s", _(qSdhusb));
			ok = false;
		}
		if (ok && wife1 && wife2 && nestr(nval(wife1), nval(wife2))) {
			msg_error("%s", _(qSdwife));
			ok = false;
		}
		if (!ok) {
			joinFamily(fam1, fref1, husb1, wife1, chil1, rest1);
			joinFamily(fam2, fref2, husb2, wife2, chil2, rest2);
			return NULL;
		}
	}

/* Create merged file with both families together */
	ASSERT(fp = fopen(editfile, DEWRITETEXT));
	fam3 = copyNodes(fam2, true, true);
	fref3 = unionNodes(fref1, fref2, true, true);
	husb3 = unionNodes(husb1, husb2, true, true);
	wife3 = unionNodes(wife1, wife2, true, true);
	rest3 = unionNodes(rest1, rest2, true, true);
	chil3 = sort_children(chil1, chil2);
	write_nodes(0, fp, ttmo, fam3, true, true, true);
	write_nodes(1, fp, ttmo, fref3, true, true, true);
	write_nodes(1, fp, ttmo, husb3, true, true, true);
	write_nodes(1, fp, ttmo, wife3, true, true, true);
	write_nodes(1, fp, ttmo, rest3, true, true, true);
	write_nodes(1, fp, ttmo, chil3, true, true, true);
	fclose(fp);

/* Have user edit merged family */
	joinFamily(fam3, fref3, husb3, wife3, rest3, chil3);
	do_edit();
	while (true) {
		fam4 = file_to_node(editfile, ttmi, &msg, &emp);
		if (!fam4 && !emp) {
			if (ask_yes_or_no_msg(msg, _(qSfredit))) {
				do_edit();
				continue;
			} 
			break;
		}
		/* check validation & allow user to reedit if invalid */
		/* this is a showstopper, so alternative is to abort */
		if (!valid_fam_tree(fam4, &msg, fam3, currentDatabase)) {
			if (ask_yes_or_no_msg(_(qSbadata), _(qSiredit))) {
				do_edit();
				continue;
			}
			freeGNodes(fam4);
			fam4 = NULL;
			break;
		}
		break;
	}
	freeGNodes(fam3);

/* Have user confirm changes */

	if (!fam4 || !ask_yes_or_no(_(qScffmrg))) {
		if (fam4) freeGNodes(fam4);
		joinFamily(fam1, fref1, husb1, wife1, chil1, rest1);
		joinFamily(fam2, fref2, husb2, wife2, chil2, rest2);
		return NULL;
	}
	splitFamily(fam4, &fref4, &husb4, &wife4, &chil4, &rest4);

 /* Modify links between persons and families */
#define CHUSB 1
#define CWIFE 2
#define CCHIL 3
	merge_fam_links(fam1, fam2, husb1, husb2, CHUSB);
	merge_fam_links(fam1, fam2, wife1, wife2, CWIFE);
	merge_fam_links(fam1, fam2, chil1, chil2, CCHIL);

/* Update database with second family; remove first */
	joinFamily(fam4, fref2, husb2, wife2, chil2, rest2);
	freeGNodes(fam4);
	nchild(fam1) = NULL;
	remove_empty_fam(fam1, currentDatabase); /* TO DO - can this fail ? 2001/11/08, Perry */
	freeGNodes(husb1);
	freeGNodes(wife1);
	freeGNodes(chil1);
	freeGNodes(rest1);
	joinFamily(fam2, fref4, husb4, wife4, chil4, rest4);
	resolve_refn_links(fam2);
	fam_to_dbase(fam2);

/* sanity check lineage links */
	check_fam_lineage_links(fam2);
	
	return node_to_record(fam2);
}
/*================================================================
 * merge_fam_links -- Shift links of persons in list1 from fam1 to
 *   fam2.  List1 holds the persons that refer to fam1, and list2
 *   holds the persons who refer to fam2.  If a person is on both
 *   lists, the reference in the person to the fam1 is removed from
 *   the person.  If a person is only on list1, the reference to fam1
 *   is changed to refer to fam2.  No changes are made for persons
 *   only on list2.  No changes are made to the references from the
 *   families to the persons.
 *================================================================*/
void
merge_fam_links (GNode *fam1, GNode *fam2, GNode *list1, GNode *list2, int code)
{
	GNode *curs1, *curs2, *prev, *this, *next, *first, *keep=NULL;
	GNode *indi, *name, *refn, *sex, *body, *famc, *fams;

	curs1 = list1;
	while (curs1) {
		curs2 = list2;
		while (curs2 && nestr(nval(curs1), nval(curs2)))
			curs2 = nsibling(curs2);
		indi = key_to_indi(rmvat(nval(curs1)));
		splitPerson(indi, &name, &refn, &sex, &body, &famc, &fams);
		prev = NULL;
		if (code == CHUSB || code == CWIFE)
			first = this = fams;
		else
			first = this = famc;

/* Both fams linked to this indi; remove link in indi to first */

		if (curs2) {
			while (this) {
				if (eqstr(nval(this), nxref(fam1))) {
					next = nsibling(this);
					nsibling(this) = NULL;
					keep = nchild(this);
					free_node(this,"merge_fam_links");
					if (!prev)
						first = next;
					else
						nsibling(prev) = next;
					this = next;
				} else {
					prev = this;
					this = nsibling(this);
				}
			}
			this = first;
			while (keep && this) {
				if (eqstr(nval(this), nxref(fam2))) {
					nchild(this) =
					    unionNodes(nchild(this), keep,
					    true, false);
/*HERE*/
				}
				this = nsibling(this);
			}

/* Only first fam linked with this indi; move link to second */

		} else {
			while (this) {
				if (eqstr(nval(this), nxref(fam1))) {
					stdfree(nval(this));
					nval(this) = strsave(nxref(fam2));
				}
				prev = this;
				this = nsibling(this);
			}
		}
		if (code == CHUSB || code == CWIFE)
			fams = first;
		else
			famc = first;
		joinPerson(indi, name, refn, sex, body, famc, fams);
		indi_to_dbase(indi);
		curs1 = nsibling(curs1);
	}
}
/*================================================
 * sort_children -- Return sorted list of children
 *==============================================*/
static GNode *
sort_children (GNode *chil1,
               GNode *chil2)
{
	GNode *copy1, *copy2, *chil3, *prev, *kid1, *kid2;
	String year1, year2;
	int int1, int2;
	/* copy1 contains all children in chil1 not in chil2 */
	copy1 = remove_dupes(chil1, chil2);
	copy2 = copyNodes(chil2, true, true);
	int1 = int2 = 1;
	prev = chil3 = NULL;
	while (copy1 && copy2) {
		if (int1 == 1) {
			kid1 = key_to_indi(rmvat(nval(copy1)));
			year1 = eventToDate(BIRT(kid1), true);
			if (!year1)
				year1 = eventToDate(BAPT(kid1), true);
			int1 = year1 ? atoi(year1) : 0;
		}
		if (int2 == 1) {
			kid2 = key_to_indi(rmvat(nval(copy2)));
			year2 = eventToDate(BIRT(kid2), true);
			if (!year2)
				year2 = eventToDate(BAPT(kid2), true);
			int2 = year2 ? atoi(year2) : 0;
		}
		if (int1 < int2) {
			if (!prev)
				prev = chil3 = copy1;
			else
				prev = nsibling(prev) = copy1;
			copy1 = nsibling(copy1);
			int1 = 1;
		} else {
			if (!prev)
				prev = chil3 = copy2;
			else
				prev = nsibling(prev) = copy2;
			copy2 = nsibling(copy2);
			int2 = 1;
		}
	}
	if (copy1) {
		if (!prev)
			chil3 = copy1;
		else
			nsibling(prev) = copy1;
	}
	if (copy2) {
		if (!prev)
			chil3 = copy2;
		else
			nsibling(prev) = copy2;
	}
	return chil3;
}
/*=================================================
 * remove_dupes -- Return all in list1 not in list2
 *  Creates a copy of list1 then traverses it
 *   removing any items which are in list2
 *  Then returns this new trimmed list.
 * This is pretty inefficient algorithm, as every item
 *  in list1 is compared against every item in list2
 * It would be more efficient to make a table of items
 *  on list2 first, and then use that to check each item
 *  in list1.
 *===============================================*/
static GNode *
remove_dupes (GNode *list1, GNode *list2)
{
	GNode *copy1 = copyNodes(list1, true, true);
	GNode *prev1, *next1, *curs1, *curs2;
	prev1 = NULL;
	curs1 = copy1;
	while (curs1) {
		curs2 = list2;
		while (curs2 && nestr(nval(curs1), nval(curs2)))
			curs2 = nsibling(curs2);
		if (curs2) {
			next1 = nsibling(curs1);
			nsibling(curs1) = NULL;
			freeGNodes(curs1);
			if (!prev1)
				copy1 = next1;
			else
				nsibling(prev1) = next1;
			curs1 = next1;
		} else {
			prev1 = curs1;
			curs1 = nsibling(curs1);
		}
	}
	return copy1;
}
/*=================================================
 * check_indi_lineage_links -- Check all families of
 *  this person to make sure they point back to this person
 *===============================================*/
static void
check_indi_lineage_links (GNode *indi)
{
	GNode *name=0, *refn=0, *sex=0, *body=0, *famc=0, *fams=0;
	GNode *curs=0; /* for travesing node lists */
	int bucket_index = 0;
	int element_index = 0;
	IntegerElement *element;

	IntegerTable *memtab = createIntegerTable(num_linkage_buckets);
	CString famkey=0; /* used inside traversal loops */
	int count=0;
	CString ikey = nxref(indi);

	/* sanity check record is not deleted */
	ASSERT(isKeyInUse(ikey));

/* Now validate lineage links of this person */
	splitPerson(indi, &name, &refn, &sex, &body, &famc, &fams);

	/*
	Make table listing all families this person is spouse in
	(& how many times each)
	*/
	for (curs = fams; curs; curs = nsibling(curs)) {
		famkey = rmvat(nval(curs));
		if (!eqstr(ntag(curs), "FAMS")) {
			char msg[512];
			snprintf(msg, sizeof(msg), _("Bad spouse tag: %s"), ntag(curs));
			fatal(msg);
		}
		incrIntegerTable(memtab, famkey);
	}

	/*
	Check that all listed families contain person as spouse as many times
	as expected
	*/
	for (element = (IntegerElement *)firstInHashTable (memtab, &bucket_index, &element_index);
	     element;
	     element = (IntegerElement *)nextInHashTable (memtab, &bucket_index, &element_index)) {
	        famkey = element->key;
	        count = element->value;
		GNode *fam = key_to_fam(famkey);
		/*
		count how many times our main person (ikey)
		occurs in this family (fam) as a spouse (HUSB or WIFE)
		*/
		int occur = 0;
		for (curs = nchild(fam); curs; curs = nsibling(curs)) {
			if (eqstr(ntag(curs), "HUSB") || eqstr(ntag(curs), "WIFE")) {
				if (eqstr(nval(curs), ikey)) {
					++occur;
				}
			}
		}
		if (count != occur) {
			char msg[512];
			snprintf(msg, sizeof(msg)
				, _("Mismatched lineage spouse links between %s and %s: " FMT_INT " and " FMT_INT)
				, ikey, famkey, count, occur);
			fatal(msg);
		}
	}
	destroy_table(memtab);
	memtab = createIntegerTable(num_linkage_buckets);

	/*
	Make table listing all families this person is child in
	(& how many times each)
	*/
	for (curs = famc; curs; curs = nsibling(curs)) {
		famkey = rmvat(nval(curs));
		if (!eqstr(ntag(curs), "FAMC")) {
			char msg[512];
			snprintf(msg, sizeof(msg), _("Bad child tag: %s"), ntag(curs));
			fatal(msg);
		}
		incrIntegerTable(memtab, famkey);
	}

	/*
	Check that all listed families contain person as child (CHIL) as many times
	as expected
	*/
	for (element = (IntegerElement *)firstInHashTable (memtab, &bucket_index, &element_index);
	     element;
	     element = (IntegerElement *)nextInHashTable (memtab, &bucket_index, &element_index)) {
	        famkey = element->key;
	        count = element->value;
		GNode *fam = key_to_fam(famkey);
		/*
		count how many times our main person (ikey)
		occurs in this family (fam) as a child (CHIL)
		*/
		int occur = 0;
		for (curs = nchild(fam); curs; curs = nsibling(curs)) {
			if (eqstr(ntag(curs), "CHIL")) {
				if (eqstr(nval(curs), ikey)) {
					++occur;
				}
			}
		}
		if (count != occur) {
			char msg[512];
			snprintf(msg, sizeof(msg)
				, _("Mismatched lineage child links between %s and %s: " FMT_INT " and " FMT_INT)
				, ikey, famkey, count, occur);
			fatal(msg);
		}
	}

	joinPerson(indi, name, refn, sex, body, famc, fams);
	destroy_table(memtab);
}
/*=================================================
 * check_fam_lineage_links -- Check all persons of
 *  this family to make sure they point back to this family
 *===============================================*/
static void
check_fam_lineage_links (GNode *fam)
{
	GNode *fref=0, *husb=0, *wife=0, *chil=0, *rest=0;
	GNode *curs=0; /* for travesing node lists */
	int bucket_index = 0;
	int element_index = 0;
	IntegerElement *element;

	IntegerTable *memtab = createIntegerTable(num_linkage_buckets);
	CString indikey=0; /* used inside traversal loops */
	int count=0;
	CString fkey = nxref(fam);

	/* sanity check record is not deleted */
	ASSERT(isKeyInUse(fkey));
	
/* Now validate lineage links of this family */
	splitFamily(fam, &fref, &husb, &wife, &chil, &rest);

	/*
	Make table listing all spouses in this family
	(& how many times each)
	*/
	for (curs = husb; curs; curs = nsibling(curs)) {
		indikey = rmvat(nval(curs));
		if (!eqstr(ntag(curs), "HUSB")) {
			char msg[512];
			snprintf(msg, sizeof(msg), _("Bad HUSB tag: %s"), ntag(curs));
			fatal(msg);
		}
		incrIntegerTable(memtab, indikey);
	}
	for (curs = wife; curs; curs = nsibling(curs)) {
		indikey = rmvat(nval(curs));
		if (!eqstr(ntag(curs), "WIFE")) {
			char msg[512];
			snprintf(msg, sizeof(msg), _("Bad HUSB tag: %s"), ntag(curs));
			fatal(msg);
		}
		incrIntegerTable(memtab, indikey);
	}

	/*
	Check that all listed persons contain family as FAMS as many times
	as expected
	*/
	for (element = (IntegerElement *)firstInHashTable (memtab, &bucket_index, &element_index);
	     element;
	     element = (IntegerElement *)nextInHashTable (memtab, &bucket_index, &element_index)) {
	        indikey = element->key;
	        count = element->value;
		GNode *indi = key_to_indi(indikey);
		/*
		count how many times our main family (fkey)
		occurs in this person (indi) as a spousal family (FAMS)
		*/
		int occur = 0;
		for (curs = nchild(indi); curs; curs = nsibling(curs)) {
			if (eqstr(ntag(curs), "FAMS")) {
				if (eqstr(nval(curs), fkey)) {
					++occur;
				}
			}
		}
		if (count != occur) {
			char msg[512];
			snprintf(msg, sizeof(msg)
				, _("Mismatched lineage spouse links between %s and %s: " FMT_INT " and " FMT_INT)
				, fkey, indikey, count, occur);
			fatal(msg);
		}
	}
	destroy_table(memtab);
	memtab = createIntegerTable(num_linkage_buckets);

	/*
	Make table listing all families this person is child in
	(& how many times each)
	*/
	for (curs = chil; curs; curs = nsibling(curs)) {
		indikey = rmvat(nval(curs));
		if (!eqstr(ntag(curs), "CHIL")) {
			char msg[512];
			snprintf(msg, sizeof(msg), _("Bad child tag: %s"), ntag(curs));
			fatal(msg);
		}
		incrIntegerTable(memtab, indikey);
	}

	/*
	Check that all listed families contain person as FAMC as many times
	as expected
	*/
	for (element = (IntegerElement *)firstInHashTable (memtab, &bucket_index, &element_index);
	     element;
	     element = (IntegerElement *)nextInHashTable (memtab, &bucket_index, &element_index)) {
	        indikey = element->key;
	        count = element->value;
		GNode *indi = key_to_indi(indikey);
		/*
		count how many times our main family (fkey)
		occurs in this person (indi) as a parental family (FAMC)
		*/
		int occur = 0;
		for (curs = nchild(indi); curs; curs = nsibling(curs)) {
			if (eqstr(ntag(curs), "FAMC")) {
				if (eqstr(nval(curs), fkey)) {
					++occur;
				}
			}
		}
		if (count != occur) {
			char msg[512];
			snprintf(msg, sizeof(msg)
				, _("Mismatched lineage child links between %s and %s: " FMT_INT " and " FMT_INT)
				, fkey, indikey, count, occur);
			fatal(msg);
		}
	}
	
	
	joinFamily(fam, fref, husb, wife, chil, rest);
}

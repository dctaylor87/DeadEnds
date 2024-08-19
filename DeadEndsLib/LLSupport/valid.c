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
/*=============================================================
 * valid.c -- Record validation functions
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 02 Sep 93
 *   3.0.0 - 11 Sep 94    3.0.2 - 13 Dec 94
 *   3.0.3 - 23 Jul 96
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif


#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "standard.h"
#include "llnls.h"

#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "refnindex.h"
#include "gnode.h"
#include "readwrite.h"
#include "splitjoin.h"
#include "rfmt.h"
#include "xlat.h"
#include "editing.h"
#include "messages.h"
#include "refns.h"
#include "locales.h"
#include "lloptions.h"

/*===================================
 * valid_indi_tree -- Validate person tree
 *  indi1:  [IN]  person to validate
 *  pmsg:   [OUT] error message, if any
 *  orig:   [IN]  person to match - may be NULL
 * rtn: false for bad
 * Should be replaced by valid_indi(RECORD,...) ?
 *=================================*/
bool
valid_indi_tree (GNode *indi1, String *pmsg, GNode *orig, Database *database)
{
	GNode *refn;
	GNode *name1, *refn1, *sex1, *body1, *famc1, *fams1, *node;
	GNode *name0, *refn0, *sex0, *body0, *famc0, *fams0;
	SexType isex;
	int num;
	String *keys, ukey;

	if (!indi1) {
		*pmsg = _(qSbademp);
  		return false;
	}
	if (nestr("INDI", ntag(indi1))) {
		*pmsg = _(qSbadin0);
		return false;
	}
	if (nsibling(indi1)) {
		*pmsg = _(qSbadmul);
		return false;
	}
	splitPerson(indi1, &name1, &refn1, &sex1, &body1, &famc1, &fams1);
	if (getdeoptint("RequireNames", 0) && !name1) {
		*pmsg = _("This person record does not have a name line.");
		goto bad2;
	}
	for (node = name1; node; node = nsibling(node)) {
		if (!valid_name(nval(node))) {
			*pmsg = _(qSbadenm);
			goto bad2;
		}
	}
	name0 = refn0 = sex0 = body0 = famc0 = fams0 = NULL;
	if (orig)
		splitPerson(orig, &name0, &refn0, &sex0, &body0, &famc0,
		    &fams0);
	if (orig && !iso_nodes(indi1, orig, false, false)) {
		*pmsg = _(qSbadind); 
		goto bad1;
	}
	if (!iso_nodes(famc1, famc0, false, true)) {
		*pmsg = _(qSbadfmc);
		goto bad1;
	}
	if (!iso_nodes(fams1, fams0, false, true)) {
		*pmsg = _(qSbadfms); 
		goto bad1;
	}
	isex = valueToSex(sex0);
	if (!fams0) isex = sexUnknown;
	if (isex != sexUnknown && isex != valueToSex(sex1)) {
		*pmsg = _(qSbadparsex);
		goto bad1;
	}
#if defined(DEADENDS)
	/* if there is more than one record with the REFN, then the
	   database is broken -- while a record can have an arbitrary
	   number of REFNs, each REFN *MUST* be unique. */
	if (database) {
	  /* we can only check REFNs if we have a database */
	  for (refn = refn1; refn != NULL; refn = nsibling(refn)) {
	    ukey = nval(refn);
	    CString key = getRefn (ukey, database);
	    if (! key || ! orig || nestr(key, rmvat(nxref(indi1)))) {
	      *pmsg = _(qSbadirefn);
	      goto bad1;
	    }
	  }
	}
#else
	/* if there are more than one refn should check each */
	for (refn = refn1; refn != NULL; refn = nsibling(refn)) {
		ukey = nval(refn);
		get_refns(ukey, &num, &keys, 'I');
		if (num > 1 || (num == 1 && (!orig ||
			nestr(keys[0], rmvat(nxref(indi1)))))) {
			*pmsg = _(qSbadirefn);
			goto bad1;
		}
	}
#endif
	if (orig)
		joinPerson(orig, name0, refn0, sex0, body0, famc0, fams0);
	joinPerson(indi1, name1, refn1, sex1, body1, famc1, fams1);
	return true;
bad1:
	if (orig)
		joinPerson(orig, name0, refn0, sex0, body0, famc0, fams0);
bad2:
	joinPerson(indi1, name1, refn1, sex1, body1, famc1, fams1);
	return false;
}
/*===============================
 * valid_fam_tree -- Validate FAM tree
 *  fam1,  [IN]  family to validate
 *  pmsg:  [OUT] error message, if any
 *  fam0:  [IN]  family to match - may be NULL
 * Should be replaced by valid_fam(RECORD,...) ?
 *=============================*/
bool
valid_fam_tree (GNode *fam1, String *pmsg, GNode *fam0, Database *database)
{
	GNode *refn0, *husb0, *wife0, *chil0, *body0;
	GNode *refn1, *husb1, *wife1, *chil1, *body1;

	if (!fam1) {
		*pmsg = _(qSbademp);
  		return false;
	}
	if (nestr("FAM", ntag(fam1))) {
		*pmsg = _(qSbadfm0);
		return false;
	}
	if (nsibling(fam1)) {
		*pmsg = _(qSbadmul);
		return false;
	}

	refn0 = husb0 = wife0 = chil0 = body0 = NULL;
	if (fam0)
		splitFamily(fam0, &refn0, &husb0, &wife0, &chil0, &body0);
	splitFamily(fam1, &refn1, &husb1, &wife1, &chil1, &body1);
	
	if (fam0 && !iso_nodes(fam1, fam0, false, true)) {
		*pmsg = _(qSbadfam); 
		goto bad3;
	}
	if (!iso_nodes(husb1, husb0, false, true)) {
		*pmsg = _(qSbadhsb);
		goto bad3;
	}
	if (!iso_nodes(wife1, wife0, false, true)) {
		*pmsg = _(qSbadwif);
		goto bad3;
	}
	if (!iso_nodes(chil1, chil0, false, true)) {
		*pmsg = _(qSbadchl);
		goto bad3;
	}
	if (fam0)
		joinFamily(fam0, refn0, husb0, wife0, chil0, body0);
	joinFamily(fam1, refn1, husb1, wife1, chil1, body1);
	return true;
bad3:
	if (fam0)
		joinFamily(fam0, refn0, husb0, wife0, chil0, body0);
	joinFamily(fam1, refn1, husb1, wife1, chil1, body1);
	return false;
}
/*============================
 * valid_name -- Validate name
 *==========================*/
bool
valid_name (String name)
{
	int c, n = 0;
	if (!name) return false;
	if (pointer_value(name)) return false;
	while ((c = *name++)) {
		if (c == NAMESEP) n++;
	}
	return n <= 2;
}
/*======================================
 * valid_node_type -- Validate top-level node tree
 *  node:   [IN]  node to validate
 *  ntype:  [IN]  I/F/S/E/X
 *  pmsg,   [OUT] error message, if any
 *  orig:   [IN]  node to match (may be null)
 *====================================*/
bool
valid_node_type (GNode *node, char ntype, String *pmsg, GNode *node0, Database *database)
{
	switch(ntype) {
	case 'I': return valid_indi_tree(node, pmsg, node0, database);
	case 'F': return valid_fam_tree(node, pmsg, node0, database);
	case 'S': return valid_sour_tree(node, pmsg, node0, database);
	case 'E': return valid_even_tree(node, pmsg, node0, database);
	default: return valid_othr_tree(node, pmsg, node0, database);
	}
}
/*======================================
 * valid_sour_tree -- Validate SOUR tree
 *  node:  [IN]  source to validate 
 *  pmsg:  [OUT] error message, if any 
 *  orig:  [IN]  SOUR node to match 
 *====================================*/
bool
valid_sour_tree (GNode *node, String *pmsg, ATTRIBUTE_UNUSED GNode *orig, Database *database)
{
	*pmsg = NULL;
	if (!node) {
		*pmsg = _(qSbademp);
  		return false;
	}
	if (nestr("SOUR", ntag(node))) {
		*pmsg = _(qSbadsr0);
		return false;
	}
#if 0
	/* validation unimplemented */
	if (orig)
	{
		*pmsg = _(qSbadsr0);
		return false;
	}
#endif
	return true;
}
/*======================================
 * valid_even_tree -- Validate EVEN tree
 *  node:  [IN]  source to validate
 *  pmsg,  [OUT] error message, if any
 *  orig:  [IN]  EVEN node to match
 *====================================*/
bool
valid_even_tree (GNode *node, String *pmsg, ATTRIBUTE_UNUSED GNode *orig, Database *database)
{
	*pmsg = NULL;
	if (!node) {
		*pmsg = _(qSbademp);
  		return false;
	}
	if (nestr("EVEN", ntag(node))) {
		*pmsg = _(qSbadev0);
		return false;
	}
#if 0
	/* validation unimplemented */
	if (orig)
	{
		*pmsg = _(qSbadev0);
		return false;
	}
#endif
	return true;
}
/*======================================
 * valid_othr_tree -- Validate OTHR tree
 *  node:  [IN]  source to validate
 *  pmsg,  [OUT] error message, if any
 *  orig:  [IN]  OTHR node to match
 *====================================*/
bool
valid_othr_tree (GNode *node, String *pmsg, ATTRIBUTE_UNUSED GNode *orig, Database *database)
{
	*pmsg = NULL;
	if (!node) {
		*pmsg = _(qSbademp);
  		return false;
	}
	if (eqstr("INDI", ntag(node)) || eqstr("FAM", ntag(node))
		|| eqstr("SOUR", ntag(node)) || eqstr("EVEN", ntag(node))) {
		*pmsg = _(qSbadothr0);
		return false;
	}
#if 0
	/* validation unimplemented */
	if (orig)
	{
		*pmsg = _(qSbadothr0);
		return false;
	}
#endif
	return true;
}
/*=========================================
 * pointer_value -- See if value is pointer
 *=======================================*/
bool
pointer_value (String val)
{
	if (!val || *val != '@' || strlen(val) < 3) return false;
	return val[strlen(val)-1] == '@';
}

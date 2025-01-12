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
 * gstrings.c -- Routines to creates child strings
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 25 Aug 93
 *   3.0.0 - 02 May 94    3.0.2 - 24 Nov 94
 *   3.0.3 - 15 Aug 95
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
//#include "ll-porting.h"
#include "standard.h"
#include "denls.h"

#include "list.h"
#include "zstr.h"
#include "refnindex.h"
#include "gnode.h"
#include "database.h"
#include "rfmt.h"
#include "translat.h"
#include "messages.h"
#include "lineage.h"
#include "de-strings.h"
#include "codesets.h"
#include "xlat.h"
#include "readwrite.h"
#include "splitjoin.h"
#include "ll-node.h"
#include "gstrings.h"
#include "locales.h"
#include "lloptions.h"

/*********************************************
 * local variables
 *********************************************/

static int nchil = 0, maxchil = 0;
static String *chstrings = NULL, *chkeys = NULL;

static bool displaykeys=true;

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*===================================================================
 * get_child_strings -- Return children strings; each string has name
 *   and event info, if avail  
 *  fam:   [in] family of interest
 *  rfmt:  [in] reformatting functions
 *  pnum:  [out] number of output strings
 *  pkeys: [out] array of output strings (children descriptions)
 *=================================================================*/
String *
get_child_strings (GNode *fam, bool rfmt, int *pnum, String **pkeys, Database *database)
{
	GNode *chil;
	int i;

	for (i = 0; i < nchil; i++) {
		stdfree(chstrings[i]);
		stdfree(chkeys[i]);
	}
	nchil = *pnum = 0;
	if (!fam || !(chil = CHIL(fam))) return NULL;
	nchil = gNodesLength(chil);
	if (nchil == 0) return NULL;
	if (nchil > (maxchil - 1)) {
		if (maxchil) {
			stdfree(chstrings); 
			stdfree(chkeys); 
		}
		chstrings = (String *) stdalloc((nchil+5)*sizeof(String));
		chkeys = (String *) stdalloc((nchil+5)*sizeof(String));
		maxchil = nchil + 5;
	}
	FORCHILDREN(fam,child,key, i, database->recordIndex)
		chstrings[i-1] = indi_to_list_string(child, NULL, 66, rfmt, true);
		chkeys[i-1] = strsave(nxref(child));
	ENDCHILDREN
	*pnum = nchil;
	*pkeys = chkeys;
	return chstrings;
}
/*================================================
 * indi_to_list_string -- Return menu list string.
 *  returns heap-alloc'd string
 *  indi:   [IN]  source person
 *  fam:    [IN]  relevant family (used in spouse lists)
 *  len:    [IN]  max length desired
 *  rfmt:   [IN]  reformating functions (may be NULL)
 *  appkey: [IN]  allow appending key ?
 *==============================================*/
String
indi_to_list_string (GNode *indi, GNode *fam, int len, bool rfmt, bool appkey)
{
	char scratch[MAXLINELEN];
	int linelen = MAXLINELEN;
	String name, evt = NULL, p = scratch;
	int hasparents;
	int hasfamily;
	if (len>linelen)
		len = linelen;
	if (indi) {
		ASSERT(name = personToName(indi, len));
	} else
		name = _(qSunksps);
	snprintf(p, linelen, "%s", name);
	linelen -= strlen(p);
	p += strlen(p);

	if (fam)
	  evt = familyToEvent(fam, "MARR", _(qSdspa_mar), len, rfmt);
	if (!evt)
	  evt = personToEvent(indi, "BIRT", _(qSdspa_bir), len, rfmt);
	if (!evt)
	  evt = personToEvent(indi, "CHR", _(qSdspa_chr), len, rfmt);
	if (!evt)
	  evt = personToEvent(indi, "DEAT", _(qSdspa_dea), len, rfmt);
	if (!evt)
	  evt = personToEvent(indi, "BURI", _(qSdspa_bur), len, rfmt);

	if (evt) {
		snprintf(p, linelen, ", %s", evt);
		linelen -= strlen(p);
		ASSERT(linelen > 0);
		p += strlen(p);
	}
	if (appkey && indi && displaykeys) {
		if (getdeoptint("DisplayKeyTags", 0) > 0) {
			snprintf(p, linelen, " (i%s)", nxref(indi));
		} else {
			snprintf(p, linelen, " (%s)", nxref(indi));
		}
		linelen -= strlen(p);
		ASSERT(linelen > 0);
		p += strlen(p);
	}
	if (appkey && fam && displaykeys) {
		if (getdeoptint("DisplayKeyTags", 0) > 0) {
			snprintf(p, linelen, " (f%s)", nxref(fam));
		} else {
			snprintf(p, linelen, " (%s)", nxref(fam));
		}
		linelen -= strlen(p);
		ASSERT(linelen > 0);
		p += strlen(p);
	}
	if(indi) {
	    if(FAMC(indi)) hasparents = 1;
	    else hasparents = 0;
	    if(FAMS(indi)) hasfamily = 1;
	    else hasfamily = 0;
	    if(hasfamily || hasparents) {
		ASSERT(linelen > 5);
		char *with_p_fam    = (hasfamily ? "PS" : "P");
		char *without_p_fam = (hasfamily ? "S"  : "" );
		char *value = (hasparents ? with_p_fam : without_p_fam);
		snprintf(p, linelen, " [%s]", value);
		linelen -= (3 + strlen (value));
		ASSERT(linelen > 0);
	    }
	}
	limit_width(scratch, len, uu8);
	return strsave(scratch);
}
/*================================================
 * sour_to_list_string -- Return menu list string.
 * Created: 2000/11/29, Perry Rapp
 *==============================================*/
String
sour_to_list_string (GNode *sour, int len, String delim)
{
	char scratch[1024];
	String name, p=scratch;
	int mylen=len;
	if (mylen>(int)sizeof(scratch))
		mylen=sizeof(scratch);
	p[0]=0;
	destrcatn(&p, "(S", &mylen);
	destrcatn(&p, nxref(sour)+1, &mylen);
	destrcatn(&p, ") ", &mylen);
	name = node_to_tag(sour, "REFN", len);
	if (name)
		destrcatn(&p, name, &mylen);
	name = node_to_tag(sour, "TITL", len);
	if (name && mylen > 20)
	{
		destrcatn(&p, delim, &mylen);
		destrcatn(&p, name, &mylen);
	}
	name = node_to_tag(sour, "AUTH", len);
	if (name && mylen > 20)
	{
		destrcatn(&p, delim, &mylen);
		destrcatn(&p, name, &mylen);
	}
	limit_width(scratch, len, uu8);
	return strsave(scratch);
}
/*================================================
 * even_to_list_string -- Return menu list string.
 * Created: 2001/12/16, Perry Rapp
 *==============================================*/
String
even_to_list_string (GNode *even, int len, ATTRIBUTE_UNUSED String delim)
{
	char scratch[1024];
	String name, p=scratch;
	int mylen=len;
	if (mylen>(int)sizeof(scratch))
		mylen=sizeof(scratch);
	p[0]=0;
	destrcatn(&p, "(E", &mylen);
	destrcatn(&p, nxref(even)+1, &mylen);
	destrcatn(&p, ") ", &mylen);
	name = node_to_tag(even, "NAME", len);
	if (name)
		destrcatn(&p, name, &mylen);
        name = node_to_tag(even, "REFN", len);
        if (name) {
		destrcatn(&p, " (", &mylen);
                destrcatn(&p, name, &mylen);
		destrcatn(&p, ")", &mylen);
	}
	limit_width(scratch, len, uu8);
	return strsave(scratch);
}
/*================================================
 * fam_to_list_string -- Return menu list string.
 * Created: 2001/02/17, Perry Rapp
 *==============================================*/
String
fam_to_list_string (GNode *fam, int len, String delim, Database *database)
{
	char scratch[1024];
	String name, p=scratch;
	String tempname;
	int mylen=len;
	char counts[FMT_INT_LEN+2+FMT_INT_LEN+2+FMT_INT_LEN+2+1];
	int husbands=0, wives=0, children=0;
	int templen=0;
	GNode *refn, *husb, *wife, *chil, *rest, *node;
	if (mylen>(int)sizeof(scratch))
		mylen=sizeof(scratch);
	p[0]=0;
	destrcatn(&p, "(F", &mylen);
	destrcatn(&p, nxref(fam)+1, &mylen);
	destrcatn(&p, ")", &mylen);
	name = node_to_tag(fam, "REFN", len);
	if (name) {
		destrcatn(&p, " ", &mylen);
		destrcatn(&p, name, &mylen);
	}
	splitFamily(fam, &refn, &husb, &wife, &chil, &rest);
	for (node=husb; node; node=nsibling(node))
		husbands++;
	for (node=wife; node; node=nsibling(node))
		wives++;
	for (node=chil; node; node=nsibling(node))
		children++;
	snprintf(counts, sizeof(counts), FMT_INT "h," FMT_INT "w," FMT_INT "ch", husbands, wives, children);
	destrcatn(&p, " ", &mylen);
	destrcatn(&p, counts, &mylen);
	if (husbands) {
		node = keyToPerson(nval(husb), database->recordIndex);
		if (node) {
			destrcatn(&p, delim, &mylen);
			if (wives)
				templen = (mylen-4)/2;
			else
				templen = mylen;
			tempname = personToName(node, templen);
			limit_width(tempname, templen, uu8);
			destrcatn(&p, tempname, &mylen);
			if (wives)
				destrcatn(&p, " m. ", &mylen);
		}
	}
	if (wives) {
		node = keyToPerson(nval(wife), database->recordIndex);
		if (node) {
			if (!templen)
				templen = mylen;
			/* othewise we set templen above */
			destrcatn(&p, personToName(node, templen), &mylen);
		}
	}
	joinFamily(fam, refn, husb, wife, chil, rest);
	/* TO DO - print a husband and a wife out */
	limit_width(scratch, len, uu8);
	return strsave(scratch);
}
/*================================================
 * other_to_list_string -- Return menu list string.
 * Created: 2000/11/29, Perry Rapp
 *==============================================*/
String
other_to_list_string(GNode *node, int len, ATTRIBUTE_UNUSED String delim)
{
	char scratch[1024];
	String name, p=scratch;
	int mylen=len;
	GNode *child;
	if (mylen>(int)sizeof(scratch))
		mylen=sizeof(scratch);
	p[0]=0;
	destrcatn(&p, "(X", &mylen);
	destrcatn(&p, nxref(node)+1, &mylen);
	destrcatn(&p, ") (", &mylen);
	destrcatn(&p, ntag(node), &mylen);
	destrcatn(&p, ") ", &mylen);
	name = node_to_tag(node, "REFN", mylen);
	if (name)
		destrcatn(&p, name, &mylen);
	if (nval(node)) {
		destrcatn(&p, nval(node), &mylen);
	}
	/* append any CONC/CONT nodes that fit */
	child = nchild(node);
	while (mylen>5 && child) {
		if (!strcmp(ntag(child), "CONC")
			|| !strcmp(ntag(child), "CONT")) {
			/* skip empty CONC/CONT nodes */
			if (nval(child)) {
				destrcatn(&p, " ", &mylen);
				destrcatn(&p, nval(child), &mylen);
			}
		} else {
			break;
		}
		if (nchild(child))
			break;
		else if (nsibling(child))
			child = nsibling(child);
		else
			break;
	}
	limit_width(scratch, len, uu8);
	return strsave(scratch);
}
/*===========================================
 * generic_to_list_string -- Format a print line from
 *  a top-level node of any type
 * Caller may specify either node or key (& leave other NULL)
 *  returns heap-alloc'd string
 * Caller must specify either node or key (or both)
 * Used in lists and in extended gedcom view
 * Created: 2001/02/12, Perry Rapp
 *  node:   [IN]  node tree of indi or fam ... to be described
 *  key:    [IN]  key of record specified by node
 *  len:    [IN]  max description desired
 *  delim:  [IN]  separator to use between events
 *  rfmt:   [IN]  reformatting information
 *  appkey: [IN]  allow appending key ?
 *=========================================*/
String
generic_to_list_string (GNode *node, String key, int len, String delim,
			bool rfmt, bool appkey, Database *database)
{
	String str = 0;
	str=NULL; /* set to appropriate format */
	if (!node && key)
		node = keyToPerson (key, database->recordIndex);
	if (!key && node)
		key = nxref(node);
	if (node) {
		switch (recordType (node))
		{
		case GRPerson:
			str = indi_to_list_string(node, NULL, len, rfmt, appkey);
			break;
		case GRSource:
			str = sour_to_list_string(node, len, delim);
			break;
		case GRFamily:
			str = fam_to_list_string(node, len, delim, database);
			break;
		case GREvent:
			str = even_to_list_string(node, len, delim);
			break;
		case GROther:
			str = other_to_list_string(node, len, delim);
			break;
		default:	/* cannot happen */
			break;
		}
	}
	if (!str) {
		if (key)
			str = strsave(key);
		else
			str = strsave("??");
	}
	return str;
}
/*=======================================================
 * set_displaykeys -- Enable/disable keys in list strings
 *  That is, whether or not to show key numbers in items
 * Created: 2001/01/01, Perry Rapp
 *=====================================================*/
void
set_displaykeys (bool keyflag)
{
	displaykeys = keyflag;
}

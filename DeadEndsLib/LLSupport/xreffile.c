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

/* xreffile.c -- handles xrefs.

   In LifeLines, at startup this consulted the on-disk database to
   determine which keys were in use and which were free.  It stored,
   in memory, for each key type, a list of the free keys and the
   highest number key in use.

   In DeadEnds, by contrast, we do not currently keep track of free vs
   in-use keys.  Instead, we assume that the database, when created,
   has mostly sequential keys.  We record the highest key currently
   allocated.  When we need to allocate a new key, we allocate at the
   top, bumping the max key by one.  If this becomes a problem, we
   might someday keep a list of dead keys.

   Additionally, in LifeLines, there was only one database.  In
   DeadEnds, there can be multiple databases, so the database needs to
   be an argument.  */

/*=============================================================
 * xreffile.c -- Handle the xref file
 * This module manages the lists of free record numbers
 * For example, if I1, I2, I4, and I6 are in use
 *  then irecs contains I3 and I5 (free record numbers)
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 02 Sep 93
 *   3.0.0 - 02 May 94    3.0.2 - 10 Nov 94
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "standard.h"
#include "denls.h"
#include "sys_inc.h"

#include "list.h"
#include "zstr.h"
#include "xreffile.h"
#include "refnindex.h"
#include "gnode.h"
#include "translat.h"
#include "xlat.h"
#include "readwrite.h"
#include "de-strings.h"

/*===================================================================
 * First five words in xrefs file are number of INDI, FAM, EVEN, SOUR
 *   and other keys in file; remaining words are keys, in respective
 *   order, for the records; first in each group is next unused key;
 *   rest are keys of deleted records
 * n==1 means there are no deleted INDI keys
 * n==2 means there is one deleted INDI key (ixrefs[1])
 *=================================================================*/
/*
 In memory, data is kept in a DELETESET
 A brand new DELETE set has
   n = 1 (0 deleted records)
   recs[0] = 1 (recs[0] is always next available record above highest in use)
   max = basic allocation unit (currently 64)
   ctype varies (eg, 'I' for the INDI set)
 If there are I1-I4 and I6 are live in the database, and I5 was deleted,
 then the DELETE set has
  n = 2 (1 deleted record)
  recs[0] = 6 (next available record above highest in use)
  recs[1] = 5 (I5 was deleted, so it is available)
  max = allocation unit (still 64)
  ctype as above (eg, 'I' for the INDI set)
*/

/*********************************************
 * local types
 *********************************************/

typedef enum { DUPSOK, NODUPS } DUPS;

 /*==================================== 
 * deleteset -- set of deleted records
 *  NB: storage order is IFESX
 *  whereas canonical order is IFSEX
 *==================================*/
struct deleteset_s
{
	int32_t n; /* num keys + 1, ie, starts at 1 */
	int32_t * recs;
	int32_t max;
	char ctype;
};
typedef struct deleteset_s *DELETESET;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static bool addixref_impl(int32_t key, DUPS dups);
static bool addfxref_impl(int32_t key, DUPS dups);
static bool addsxref_impl(int32_t key, DUPS dups);
static bool addexref_impl(int32_t key, DUPS dups);
static bool addxref_impl(CString key, DUPS dups);
static bool addxxref_impl(int32_t key, DUPS dups);
static void dumpxrecs (CString type, DELETESET set, int32_t *offset);
static int32_t find_slot(int32_t keynum, DELETESET set);
static DELETESET get_deleteset_from_type(char ctype);
static String getxref(DELETESET set);
static void growxrefs(DELETESET set);
static String newxref(CString xrefp, bool flag, DELETESET set);
static bool parse_key(CString key, char * ktype, int32_t * kval);
static int xref_last(DELETESET set);

/*********************************************
 * local variables
 *********************************************/

/* INDI, FAM, EVEN, SOUR, other sets */
static struct deleteset_s irecs, frecs, srecs, erecs, xrecs;

static int xrefsize=0; /* xref file size */

static int32_t maxkeynum=-1; /* cache value of largest key extant (-1 means not sure) */

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=========================================
 * getxrefnum -- Return new keynum for type
 *  from deleted list if available, or else
 *  a new highnumber
 *  generic for all 5 types
 * Created: 2001/02/04, Perry Rapp
 *=======================================*/
static int32_t
getxrefnum (DELETESET set)
{
	int32_t keynum;
	ASSERT(set->n >= 1);
	if (set->n == 1) {
		/* no free records on list */
		/* take new high number & bump high number */
		keynum = set->recs[0]++;
	} else {
		/* take last free record on list */
		keynum = set->recs[set->n - 1];
		/* zero out the entry slipping off the top of the list */
		set->recs[set->n - 1] = 0;
		/* remove just-used entry from list */
		--(set->n);
	}
	maxkeynum=-1;
	return keynum;
}
/*=========================================
 * getxref -- Return new key pointer for type
 *  from deleted list if available, or else
 *  a new highnumber
 *  generic for all 5 types
 *=======================================*/
static String
getxref (DELETESET set)
{
	int32_t keynum = getxrefnum(set);
	static char scratch[12];
	snprintf(scratch, sizeof(scratch), "@%c" FMT_INT32 "@", set->ctype, keynum);
	return scratch;
}
/*===================================================
 * get?xref -- Wrappers for each type to getxref (qv)
 *  symmetric versions
 *=================================================*/
String getfxref (void) { return getxref(&frecs); }
String getsxref (void) { return getxref(&srecs); }
String getexref (void) { return getxref(&erecs); }
String getxxref (void) { return getxref(&xrecs); }
/*===================================================
 * get?xrefnum -- Wrappers for each type to getxrefnum (qv)
 * Created: 2001/02/04, Perry Rapp
 * MTE: 2019-01-05: Why do INDIs have this special
 * interface that exposes the internal xref type?
 * Should try to revert back to getixref as above.
 *=================================================*/
int32_t getixrefnum (void) { return getxrefnum(&irecs); }

/*================================
 * dumpxrefs -- Print xrefs to stdout
 *  storage order: IFESX
 *==============================*/
void
dumpxrefs (void)
{
	int32_t offset = 0;

        printf("NOTE: n is always the number of deleted keys PLUS ONE.\n");
        printf("NOTE: Each entry indicates the next available key value.\n\n");

	/* Dump "n" values */
	printf(FMT_INT32_HEX ": I n: " FMT_INT32_HEX " (" FMT_INT32 ")\n", offset, irecs.n, irecs.n);
	offset += sizeof(irecs.n); 
	printf(FMT_INT32_HEX ": F n: " FMT_INT32_HEX " (" FMT_INT32 ")\n", offset, frecs.n, frecs.n);
	offset += sizeof(frecs.n); 
	printf(FMT_INT32_HEX ": E n: " FMT_INT32_HEX " (" FMT_INT32 ")\n", offset, erecs.n, erecs.n);
	offset += sizeof(erecs.n); 
	printf(FMT_INT32_HEX ": S n: " FMT_INT32_HEX " (" FMT_INT32 ")\n", offset, srecs.n, srecs.n);
	offset += sizeof(srecs.n); 
	printf(FMT_INT32_HEX ": X n: " FMT_INT32_HEX " (" FMT_INT32 ")\n", offset, xrecs.n, xrecs.n);
	offset += sizeof(xrecs.n); 

	/* Dump "recs" values */
	dumpxrecs("I", &irecs, &offset);
	dumpxrecs("F", &frecs, &offset);
	dumpxrecs("E", &erecs, &offset);
	dumpxrecs("S", &srecs, &offset);
	dumpxrecs("X", &xrecs, &offset);

	/* Dump size */
	printf(FMT_INT32_HEX ": EOF (" FMT_INT32_HEX ") %s\n", offset, (int32_t)xrefsize, (offset == (int32_t)xrefsize) ? "GOOD" : "BAD");
}
/*================================
 * dumpxrecs -- Print DELETESET to stdout
 *==============================*/
static void
dumpxrecs (CString type, DELETESET set, int32_t *offset)
{
	int i;

	for (i=0; i<set->n; i++)
	{
		printf(FMT_INT32_HEX ": %s[" FMT_INT_04 "]: " FMT_INT32_HEX " (" FMT_INT32 ")\n", *offset, type, i, (set->recs)[i], (set->recs)[i]);
		*offset += (int32_t)sizeof((set->recs)[i]);
	}
}
/*=====================================
 * find_slot -- Find slot at which to add key
 *===================================*/
static int32_t
find_slot (int32_t keynum, DELETESET set)
{
	int32_t lo=1;
	int32_t hi=(set->n)-1;
	/* binary search to find where to insert key */
	while (lo<=hi) {
		int32_t md = (lo + hi)/2;
		if (keynum > (set->recs)[md])
			hi=--md;
		else if (keynum < (set->recs)[md])
			lo=++md;
		else {
			return md;
		}
	}
	return lo;
}

/*=====================================
 * add_xref_to_set_impl -- Add deleted key to xrefs.
 *  generic for all types
 *===================================*/
static bool
add_xref_to_set_impl (int32_t keynum, DELETESET set, DUPS dups)
{
	int32_t lo, i;
	if (keynum <= 0 || (set->n) < 1) {
		char msg[128];
		snprintf(msg, sizeof(msg)
			, _("Corrupt DELETESET %c"), set->ctype);
		fatal(msg);
	}
	/* special case simplification if deleting last record */
	if (keynum+1 == set->recs[0]) {
		/*
		just bump the 'next free' indicator down to 
		return this keynum next, and we don't even have to
		add this to the list
		*/
		--set->recs[0];
		return true;
	}
	if (set->n >= set->max)
		growxrefs(set);
	ASSERT(set->n < set->max);

	lo = find_slot(keynum, set);
	if (lo < set->n && (set->recs)[lo] == keynum) {
		/* key is already free */
		char msg[96];
		if (dups==DUPSOK) 
			return false;
		snprintf(msg, sizeof(msg)
			, _("Tried to add already-deleted record (" FMT_INT32 ") to xref (%c)!")
			, keynum, set->ctype);
		fatal(msg); /* deleting a deleted record! */
	}
	/* key replaces xrefs[lo] - push lo+ up */
	for (i=set->n-1; i>=lo; --i)
		(set->recs)[i+1] = (set->recs)[i];
	(set->recs)[lo] = keynum;
	(set->n)++;
	maxkeynum=-1;
	return true;
}

/*===================================================
 * add?xref_impl -- Wrappers for each type to add_xref_to_set (qv)
 *  5 symmetric versions
 *=================================================*/
static bool addixref_impl (int32_t key, DUPS dups) { return add_xref_to_set_impl(key, &irecs, dups); }
static bool addfxref_impl (int32_t key, DUPS dups) { return add_xref_to_set_impl(key, &frecs, dups); }
static bool addsxref_impl (int32_t key, DUPS dups) { return add_xref_to_set_impl(key, &srecs, dups); }
static bool addexref_impl (int32_t key, DUPS dups) { return add_xref_to_set_impl(key, &erecs, dups); }
static bool addxxref_impl (int32_t key, DUPS dups) { return add_xref_to_set_impl(key, &xrecs, dups); }
/*===================================================
 * add?xref -- Wrappers for each type to add_xref_to_set (qv)
 *  5 symmetric versions
 *=================================================*/
void addixref (int key) { addixref_impl((int32_t)key, NODUPS); }
void addfxref (int key) { addfxref_impl((int32_t)key, NODUPS); }
void addsxref (int key) { addsxref_impl((int32_t)key, NODUPS); }
void addexref (int key) { addexref_impl((int32_t)key, NODUPS); }
void addxxref (int key) { addxxref_impl((int32_t)key, NODUPS); }

/*==========================================
 * growxrefs -- Grow memory for xrefs array.
 *  generic for all types
 *========================================*/
static void
growxrefs (DELETESET set)
{
	int32_t i, m = set->max, *newp;
	if (set->max == 0)
		set->max = 64;
	while (set->max <= set->n)
		set->max = set->max << 1;
	newp = (int32_t *) stdalloc((set->max)*sizeof(int32_t));
	if (m) {
		for (i = 0; i < set->n; i++)
			newp[i] = set->recs[i];
		stdfree(set->recs);
	}
	set->recs = newp;
}

/*==========================================
 * get_deleteset_from_type -- Return deleteset
 *  of type specified
 *========================================*/
static DELETESET
get_deleteset_from_type (char ctype)
{
	switch(ctype) {
	case 'I': return &irecs;
	case 'F': return &frecs;
	case 'S': return &srecs;
	case 'E': return &erecs;
	case 'X': return &xrecs;
	}
	ASSERT(0); return 0;
}
/*==========================================
 * delete_xref_if_present -- If record is listed
 *  as free, remove it from the free list
 *========================================*/
bool
delete_xref_if_present (CString key)
{
	DELETESET set=0;
	int32_t keynum=0;
	int32_t lo=0;
	int32_t i=0;

	ASSERT(key);
	ASSERT(key[0]);
	ASSERT(key[1]);
	set = get_deleteset_from_type(key[0]);
	keynum = atoi(key + 1);
	ASSERT(keynum>0);
	lo = find_slot(keynum, set);
	if (!(lo < set->n && (set->recs)[lo] == keynum))
		return false;
	/* removing xrefs[lo] -- move lo+ down */
	for (i=lo; i+1<set->n-1; ++i)
		(set->recs)[i] = (set->recs)[i+1];
	/* zero out the entry slipping off the top of the list */
	if (set->n > 1)
		set->recs[set->n - 1] = 0;
	--(set->n);
	maxkeynum=-1;
	return true;

}
/*==========================================
 * parse_key -- Get key type (first char) and numeric value
 * parse_key("I44") => 'I', 44
 *========================================*/
static bool
parse_key (CString key, char * ktype, int32_t * kval)
{
	if (!key || !key[0] || !key[1])
		return false;
	/* convert "@I44@" to "I44" */
	if (key[0] == '@' && key[strlen(key)-1] == '@')
		key = rmvat(key);
	*ktype = key[0];
	*kval = atoi(key+1);
	return true;
}

/*==========================================================
 * xref_num_????s -- Return number of type of things in database.
 *  5 symmetric versions
 *========================================================*/
static int xref_num_set (DELETESET set)
{
	ASSERT(set);
	/* next key value less number of deleted keys */
	return (int)(set->recs[0] - set->n);
}
int xref_num_indis (void) { return xref_num_set(&irecs); }
int xref_num_fams (void)  { return xref_num_set(&frecs); }
int xref_num_sours (void) { return xref_num_set(&srecs); }
int xref_num_evens (void) { return xref_num_set(&erecs); }
int xref_num_othrs (void) { return xref_num_set(&xrecs); }
/*========================================================
 * max_????s -- Return max key number of object type in db
 * 5 symmetric versions
 *======================================================*/
static int32_t max_set (DELETESET set)
{
	return set->recs[0];
}
int32_t xref_max_indis (void) { return max_set(&irecs); }
int32_t xref_max_fams (void)  { return max_set(&frecs); }
int32_t xref_max_sours (void) { return max_set(&srecs); }
int32_t xref_max_evens (void) { return max_set(&erecs); }
int32_t xref_max_othrs (void) { return max_set(&xrecs); }
/*======================================================
 * xref_max_any -- Return largest key number of any type
 *====================================================*/
int32_t
xref_max_any (void)
{
	if (maxkeynum>=0)
		return maxkeynum;
	if (xref_max_indis() > maxkeynum)
		maxkeynum = xref_max_indis();
	if (xref_max_fams() > maxkeynum)
		maxkeynum = xref_max_fams();
	if (xref_max_sours() > maxkeynum)
		maxkeynum = xref_max_sours();
	if (xref_max_evens() > maxkeynum)
		maxkeynum = xref_max_evens();
	if (xref_max_othrs() > maxkeynum)
		maxkeynum = xref_max_othrs();
	return maxkeynum;
}
/*================================================
 * newxref -- Return original or next xref value
 * xrefp = key of the individual
 * flag = use the current key
 *  returns static buffer
 *==============================================*/
static String
newxref (CString xrefp, bool flag, DELETESET set)
{
	int32_t keynum;
	static char scratch[12];
	if(flag) {
		keynum = atoi(xrefp+1);
		if(set->n != 1)
			set->n = 1;	/* forget about deleted entries */
		if(keynum >= set->recs[0])
			set->recs[0] = keynum+1;	/* next available */
		snprintf(scratch, sizeof(scratch), "@%s@", xrefp);
		return(scratch);
	}
	return(getxref(set));
}
/*================================================
 * new?xref -- Return original or next ?xref value
 * xrefp = key of the record
 * flag = use the current key
 *==============================================*/
String
newixref (CString xrefp, bool flag)
{
	return newxref(xrefp, flag, &irecs);
}
String
newfxref (CString xrefp, bool flag)
{
	return newxref(xrefp, flag, &frecs);
}
String
newsxref (CString xrefp, bool flag)
{
	return newxref(xrefp, flag, &srecs);
}
String
newexref (CString xrefp, bool flag)
{
	return newxref(xrefp, flag, &erecs);
}
String
newxxref (CString xrefp, bool flag)
{
	return newxref(xrefp, flag, &xrecs);
}

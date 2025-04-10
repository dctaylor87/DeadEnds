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
 * indiseq.c -- Person sequence operations
 * Copyright(c) 1991-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 25 Aug 93
 *   3.0.0 - 09 May 94    3.0.2 - 23 Dec 94
 *===========================================================*/
/* modified 05 Jan 2000 by Paul B. McBride (pmcbride@tiac.net) */
/* modified 2000-01-26 J.F.Chandler */
/* modified 2000-08-21 J.F.Chandler */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>
#include <stdbool.h>
#include <locale.h>

#include "porting.h"
//#include "ll-porting.h"
#include "standard.h"
#include "sequence.h"
#include "gnode.h"
#include "zstr.h"
#include "rfmt.h"
#include "de-strings.h"
#include "translat.h"
#include "xlat.h"
#include "readwrite.h"
#include "codesets.h"
#include "gstrings.h"

#include "ll-sequence.h"
#include "ask.h"
#include "ll-indiseq.h"
#include "py-set.h"		/* need to move FOR... macros elsewhere! */
#include "de-strings.h"

/*
	indiseqs are typed as to value
	as of 2001/01/07 (Perry)
	ival = ints
	pval = pointers (caller determined
	sval = strings
	null = not yet specified

	null indiseqs change type as soon as a fixed type value
	is assigned to them, but as long as only nulls are attached
	(append_indiseq_null) they remain uncommitted

	NB: null values can be appended to any type (append_indiseq_null)

	pointers are managed by the caller - in practice these are
	all PVALUES for report commands
*/


/*********************************************
 * local enums
 *********************************************/

/*====================
 * indiseq print types
 *==================*/
#define ISPRN_NORMALSEQ 0
#define ISPRN_FAMSEQ 1
#define ISPRN_SPOUSESEQ 2

/*********************************************
 * local types
 *********************************************/

#define ISize(seq)	((seq)->block.length)
#define IData(seq)	((SequenceEl **)((seq)->block.elements))

/*==================================================================
 * SequenceEl -- Data type for indiseq elements; keys are always present
 *   and belong to the structure; names are always present for
 *   persons and belong; values do not belong to the structure
 *================================================================*/

/* typedef struct tag_sortel *SORTEL; */ /* in indiseq.h */
#define skey(s) ((s)->root->key)
#define snam(s) ((s)->name)
#define sval(s) ((s)->value)
//#define sprn(s) ((s)->s_prn)
#define sprn(s) ((s)->value)	/* XXX not sure of this one! XXX */
//#define spri(s) ((s)->s_pri)

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static CString get_print_el(Sequence *, int i, int len, RFMT rfmt,
			    enum SequenceType type, Database *database);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*================================================
 * elementval_indiseq -- Return element & value from sequence
 * Created: 2000/11/29, Perry Rapp
 *  seq;   [IN]  sequence
 *  index: [IN]  index of desired element
 *  pkey:  [OUT] returned key
 *  pval:  [OUT] returned val
 *  pname: [OUT] returned name
 *==============================================*/
static bool
element_indiseq_ival (Sequence *seq, int index, CString *pkey, int *pval,
		      CString *pname)
{
	*pkey = *pname = NULL;
	if (!seq || index < 0 || index > seq->block.length - 1) return false;
	*pkey =  skey(IData(seq)[index]);
	/* do we need to allow for NUL type here ? */
	*pval = (int)(intptr_t)sval(IData(seq)[index]);
	*pname = snam(IData(seq)[index]);
	return true;
}

/*===========================================
 * generic_print_el -- Format a print line of
 *  sequence of indis
 *  returns heap-alloc'd string
 *
 *  seq:  [in] sequence containing desired element
 *  i:    [in] index of desired element
 *  len:  [in] max width description desired
 *  rfmt: [in] reformatting information
 *=========================================*/
static String
generic_print_el (Sequence *seq, int i, int len, RFMT rfmt, Database *database)
{
	CString key, name;
	elementFromSequence(seq, i, &key, &name);
	return generic_to_list_string(NULL, key, len, ", ", rfmt, true, database);
}
/*=============================================
 * spouseseq_print_el -- Format a print line of
 *  sequence of spouses
 * assume values are family keys
 *===========================================*/
static String
spouseseq_print_el (Sequence *seq, int i, int len, RFMT rfmt, Database *database)
{
	GNode *indi, *fam;
	CString key, name;
	String str;
	int val;
	if (! element_indiseq_ival(seq, i, &key, &val, &name))
	  return NULL;
	indi = keyToPerson(key, seq->index);
#if defined(DEADENDS)
	fam = keyToFamily (key, database->recordIndex);
#else
	fam = keynum_to_fam(val);
#endif
	str = indi_to_list_string(indi, fam, len, rfmt, true);
	return str;
}
/*==========================================
 * famseq_print_el -- Format a print line of
 *  sequence of families
 * assume values are spouse keys
 *========================================*/
static CString
famseq_print_el (Sequence *seq, int i, int len, RFMT rfmt, Database *database)
{
	GNode *fam=0;
	CString key=0, name=0;
	CString str=0;
	int val=0, num1=0;
	int spkeynum=0;
	Sequence *spseq = createSequence (database->recordIndex);
	GNode *indi=0;
	ZSTR zstr = zs_newn(len);

	if (! element_indiseq_ival(seq, i, &key, &val, &name))
	  return false;
	fam = keyToFamily(key, seq->index);

	/* build sequence of other spouses in this family */
	FORFAMSPOUSES(fam, spouse, seq->index)
		len++;
#if defined(DEADENDS)
		key = nxref(spouse);
#else
		key = indi_to_key(spouse);
#endif
		spkeynum = atoi(key + 1);
		if (val != spkeynum) {
			appendToSequence(spseq, key, NULL);
		}
	ENDFAMSPOUSES
	
	/* build string list of spouses */
	FORSEQUENCE(spseq, el, num)
		indi = keyToPerson(skey(el), spseq->index);
		str = indi_to_list_string(indi, fam, len/lengthSequence(spseq), rfmt, true);
		if (zs_len(zstr)) zs_apps(zstr, ", ");
		zs_apps(zstr, str);
		strfree(&str);
	ENDSEQUENCE
	
	/* make heap string to return */
	str = strdup(zs_str(zstr));
	zs_free(&zstr);
	return str;
}
/*================================================
 * get_print_el -- Get appropriate format line for
 *  one element of an indiseq
 *==============================================*/
static CString
get_print_el (Sequence *seq, int i, int len, RFMT rfmt,
	      enum SequenceType type, Database *database)
{
  CString str;
  switch (type)
    {
    case chooseTypeFamily:
      str = famseq_print_el(seq, i, len, rfmt, database);
      break;
    case chooseTypeSpouse:
      str = spouseseq_print_el(seq, i, len, rfmt, database);
      break;
    default:
      str = generic_print_el(seq, i, len, rfmt, database);
      break;
    }
  return str;
}
/*================================================
 * print_indiseq_element -- Format a print line of
 *  an indiseq (any type)
 *
 *  seq:  [in] indiseq of interest
 *  i:    [in] index of desired element
 *  buf:  [out] buffer to which to print description
 *  len:  [in] max length of buffer
 *  rfmt: [in] reformatting info
 *==============================================*/
void
print_indiseq_element (Sequence *seq, int i, String buf, int len, RFMT rfmt,
		       enum SequenceType type, Database *database)
{
	CString str;
	String ptr=buf;
	bool alloc=false;
	buf[0]='\0';
	str = sprn(IData(seq)[i]);
	if (!str) {
		/*
		 If not precomputed, make print string on-the-fly .
		 This is used for long seqs, when we don't want to keep
		 all these strings in memory all the time.
		 *
		 Note: The print_el functions return a strsave'd string.
		 It would be more efficient not to strsave. This requires
		 changing indi_to_list_string, etc.
		*/
	  str = get_print_el(seq, i, len-1, rfmt, type, database);
		alloc=true;
	}
	destrcatn(&ptr, str, &len);
	if (alloc)
		stdfree(str);
}
/*=====================================================
 * preprint_indiseq -- Preformat print lines of indiseq
 *  seq:  [in] sequence to prepare (for display)
 *  len:  [in] max line width desired
 *  rfmt: [in] reformatting info
 *===================================================*/
void
preprint_indiseq (Sequence *seq, int len, RFMT rfmt,
		  enum SequenceType type, Database *database)
{
	FORSEQUENCE(seq, el, num)
		sprn(el) = get_print_el(seq, num-1, len, rfmt, type, database);
	ENDSEQUENCE
}

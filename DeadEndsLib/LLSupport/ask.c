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
 * ask.c -- Interact with user for various reasons
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 24 Aug 93
 *   2.3.6 - 30 Oct 93    3.0.0 - 19 Aug 94
 *   3.0.2 - 02 Dec 94
 *===========================================================*/
/* modified 2000-01-26 J.F.Chandler */
/* modified 2000-04-25 J.F.Chandler */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "denls.h"
#include "sys_inc.h"

#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "hashtable.h"
#include "refnindex.h"
#include "recordindex.h"
#include "database.h"
#include "sequence.h"
#include "errors.h"
#include "ask.h"
#include "choose.h"
#include "feedback.h"
#include "rfmt.h"
#include "llinesi.h"
#include "gedcom.h"
#include "liflines.h"
#include "messages.h"
#include "codesets.h"
#include "gnode.h"
#include "database.h"
#include "de-strings.h"
#include "llpy-externs.h"
#include "ui.h"
#include "generic.h"

/*********************************************
 * external/imported variables
 *********************************************/

/*********************************************
 * local function prototypes
 *********************************************/

static GNode *ask_for_any_once(CString ttl, char ctype, ASK1Q ask1, int *prc, Database *database);
static void make_fname_prompt(String fnamebuf, int len, CString ext);

/*=====================================================
 * ask_for_fam_by_key -- Ask user to identify family by 
 *  key (or REFN)
 *  (if they enter nothing, it will fall thru to ask_for_fam)
 *  fttl: [IN]  title for prompt
 *  pttl: [IN]  title for prompt to identify spouse
 *  sttl: [IN]  title for prompt to identify sibling
 *========================================================*/
GNode *
ask_for_fam_by_key (CString fttl, CString pttl, CString sttl, Database *database)
{
  GNode *fam = ask_for_record(fttl, 'F', database);
  return fam ? fam : ask_for_fam(pttl, sttl, database);
}
/*===========================================
 * ask_for_fam -- Ask user to identify family by spouses
 *  pttl: [IN]  title for prompt to identify spouse
 *  sttl: [IN]  title for prompt to identify sibling
 *=========================================*/
GNode *
ask_for_fam (CString pttl, CString sttl, Database *database)
{
	GNode *sib=0, *prn=0;
	if (! database)
	  database = currentDatabase;

	prn = ask_for_indi(pttl, DOASK1, database);
	if (!prn)  {
		GNode *fam=0;
		GNode *frec=0;
		sib = ask_for_indi(sttl, DOASK1, database);
		if (!sib) return NULL;
		fam = FAMC(nztop(sib));
		if (!fam) {
			msg_error("%s", _(qSntchld));
			return NULL;
		}
		frec = keyToFamilyRecord(nval(fam), database);
		return frec;
	}
	if (!FAMS(nztop(prn))) {
		msg_error("%s", _(qSntprnt));
		return NULL;
	}
	return chooseFamily(prn, _(qSparadox), _(qSidfbrs), true, database);
}
/*===========================================
 * ask_for_int -- Ask user to provide integer
 * titl: [IN]  prompt title
 * TODO: change to bool return for failure
 *=========================================*/
bool
ask_for_int (CString ttl, int * prtn)
{
	int ival, c, neg;
	char buffer[MAXPATHLEN];
	while (true) {
		String p = buffer;
		if (!ask_for_string(ttl, _(qSaskint), buffer, sizeof(buffer)))
			return false;
		neg = 1;
		while (iswhite(*p++))
			;
		--p;
		if (*p == '-') {
			neg = -1;
			p++;
			while (iswhite(*p++))
				;
			--p;
		}
		if (chartype(*p) == DIGIT) {
			ival = *p++ - '0';
			while (chartype(c = *p++) == DIGIT)
				ival = ival*10 + c - '0';
			--p;
			while (iswhite(*p++))
				;
			--p;
			if (*p == 0) {
				*prtn = ival*neg;
				return true;
			}
		}
	}
}
/*======================================
 * ask_for_file_worker -- Ask for and open file
 *  ttl:       [IN]  title of question (1rst line)
 *  pfname     [OUT] file as user entered it (optional param)
 * pfname is  heap-allocated
 *====================================*/
typedef enum { INPUT, OUTPUT } DIRECTION;
static FILE *
ask_for_file_worker (CString mode,
                     CString ttl,
                     String *pfname,
                     CString path,
                     CString ext,
                     DIRECTION direction)
{
	FILE *fp;
	char prompt[MAXPATHLEN];
	char fname[MAXPATHLEN];
	int elen, flen;
	bool rtn;

	make_fname_prompt(prompt, sizeof(prompt), ext);

	if (direction==INPUT)
		rtn = ask_for_input_filename(ttl, path, prompt, fname, sizeof(fname));
	else
		rtn = ask_for_output_filename(ttl, path, prompt, fname, sizeof(fname));
	
	if (pfname) {
		if (fname[0])
			*pfname = strsave(fname);
		else
			*pfname = 0;
	}
	if (!rtn || !fname[0]) return NULL;

	if (!expandSpecialFilenameChars(fname, sizeof(fname), uu8)) {
		msg_error("%s", _(qSfn2long));
		return NULL;
	}

ask_for_file_try:

	/* try name as given */
	fp = fopenPath (fname, mode, path);
	if (fp)
		return fp;

	/* try default extension */
	if (ext) {
		elen = strlen(ext);
		flen = strlen(fname);
		if (elen<flen && path_match(fname+flen-elen, ext)) {
			ext = NULL;	/* the file name has the extension already */
		} else {
			/* add extension and go back and retry */
			destrapps(fname, sizeof(fname), uu8, ext);
			ext = NULL; /* only append extension once! */
			goto ask_for_file_try;
		}
	}

	/* failed to open it, give up */
	msg_error(_(qSnofopn), fname);
	return NULL;
}
/*======================================
 * make_fname_prompt -- Create prompt line
 *  for filename, depending on extension
 * Created: 2001/12/24, Perry Rapp
 *====================================*/
static void
make_fname_prompt (String fnamebuf, int len, CString ext)
{
	if (ISNULL(ext)) {
		ext = NULL;	/* a null extension is the same as no extension */
		destrncpyf(fnamebuf, len, uu8, "%s: ", _(qSwhtfname));
	}
	else {
		destrncpyf(fnamebuf, len, uu8, _(qSwhtfnameext), ext);
	}
}
/*======================================
 * ask_for_input_file -- Ask for and open file for input
 *  ttl:       [IN]  title of question (1rst line)
 *  pfname     [OUT] file as user entered it (optional param)
 *  pfullpath  [OUT] file as found (optional param)
 *====================================*/
FILE *
ask_for_input_file (CString mode,
                    CString ttl,
                    String *pfname,
                    CString path,
                    CString ext)
{
	return ask_for_file_worker(mode, ttl, pfname, path, ext, INPUT);
}

/*======================================
 * ask_for_output_file -- Ask for and open file for output
 *  ttl:   [IN]  title of question (1rst line)
 *  pfname [OUT] optional output parameter (pass NULL if undesired)
 *====================================*/
FILE *
ask_for_output_file (CString mode,
                     CString ttl,
                     String *pfname,
                     CString path,
                     CString ext)
{
	return ask_for_file_worker(mode, ttl, pfname, path, ext, OUTPUT);
}
	/* RC_DONE means user just hit enter -- interpret as a cancel */
#define RC_DONE       0
	/* RC_NOSELECT means user's choice couldn't be found & we gave up (& told them) */
#define RC_NOSELECT   1
	/* RC_SELECT means user chose a valid list (may have only one entry) */
#define RC_SELECT     2
/*=================================================
 * ask_for_indiseq -- Ask user to identify sequence
 *  ttl:   [IN]  prompt (title) to display
 *  ctype: [IN]  type of record (eg, 'I') (0 for any)
 *  prc:   [OUT] result code (RC_DONE, RC_SELECT, RC_NOSELECT)
 *===============================================*/
Sequence *
ask_for_indiseq (CString ttl, char ctype ATTRIBUTE_UNUSED,
		 int *prc, Database *database)
{
	while (1)
	{
		Sequence *seq=0;
		char name[MAXPATHLEN];
		*prc = RC_DONE;
		if (!ask_for_string(ttl, _(qSidbrws), name, sizeof(name)))
			return NULL;
		if (*name == 0) return NULL;
		*prc = RC_NOSELECT;
		if (eqstr(name, "@")) {
			seq = invoke_search_menu();
			if (!seq)
				continue; /* fallback to main question above */
			*prc = RC_SELECT;
		} else {
			seq = stringToSequence(name, database);
			if (seq) {
				*prc = RC_SELECT;
			} else {
				msg_error("%s", _(qSnonamky));
				continue;
			}
		}
		return seq;
	}
}
/*============================================================
 * ask_for_any_once -- Have user identify sequence and select record
 *  ttl:   [IN]  title to present
 *  ctype: [IN]  type of record (eg, 'I') (0 for any, 'B' for any preferring INDI)
 *  ask1:  [IN]  whether to present list if only one matches their desc.
 *  prc:   [OUT] result (RC_DONE, RC_SELECT, RC_NOSELECT)
 *==========================================================*/
static GNode *
ask_for_any_once (CString ttl, char ctype, ASK1Q ask1, int *prc, Database *database)
{
	GNode *indi = 0;
	Sequence *seq = ask_for_indiseq(ttl, ctype, prc, database);
	if (*prc == RC_DONE || *prc == RC_NOSELECT) return NULL;
	ASSERT(*prc == RC_SELECT);
	/* user chose a set of possible answers */
	/* might be a single-entry indiseq, but if so still need to confirm */
	ASSERT(*prc == RC_SELECT);
	if (ctype == 'I') {
		indi = chooseFromSequence(seq, ask1, _(qSifonei), _(qSnotonei), chooseTypeDefault);
	} else {
		indi = chooseFromSequence(seq, ask1, _(qSifonex), _(qSnotonex), chooseTypeDefault);
	}
	deleteSequence(seq);
	*prc = indi ? RC_SELECT : RC_NOSELECT;
	return indi;
}
/*=================================================================
 * ask_for_indi -- Ask user to identify sequence and select person;
 *   reask protocol used
 * ttl:      [in] title for question
 * ask1:     [in] whether to present list if only one matches
 *===============================================================*/
GNode *
ask_for_indi (CString ttl, ASK1Q ask1, Database *database)
{
	int rc = 0;
	GNode *indi = ask_for_any_once(ttl, 'I', ask1, &rc, database);
	return indi;
}
/*=================================================================
 * ask_for_any -- Ask user to identify sequence and select record
 *   reask protocol used
 * ttl:      [in] title for question
 * confirmq: [in] whether to confirm after choice
 * ask1:     [in] whether to present list if only one matches
 *===============================================================*/
GNode *
ask_for_any (CString ttl, ASK1Q ask1, Database *database)
{
	char ctype = 0; /* code for any type */
	while (true) {
		int rc;
		GNode *record = ask_for_any_once(ttl, ctype, ask1, &rc, database);
		if (rc == RC_DONE || rc == RC_SELECT)
			return record;
		return NULL;
	}
}
/*===================================================================
 * ask_for_indi_list -- Ask user to identify person sequence
 * reask if true if we should give them another chance if their search hits nothing
 * returns null value indiseq
 * used by both reports & interactive use
 *=================================================================*/
Sequence *
ask_for_indi_list (CString ttl, bool reask, Database *database)
{
	while (true) {
		int rc = RC_DONE;
		Sequence *seq = ask_for_indiseq(ttl, 'I', &rc, database);
		if (rc == RC_DONE)
			return NULL;
		if (rc == RC_NOSELECT) {
			if (!reask || !ask_yes_or_no(_(qSentnam)))
				return NULL;
			continue;
		}
		ASSERT(seq);
		rc = chooseListFromSequence(_(qSnotonei), seq, chooseTypeSpouse);
		if (rc == -1) {
			deleteSequence(seq);
			seq = NULL;
			if (!reask || !ask_yes_or_no(_(qSentnam)))
				return NULL;
		}
		return seq;
	}
}
/*==========================================================
 * ask_for_indi_key -- Have user identify person; return key
 *========================================================*/
String
ask_for_indi_key (CString ttl, ASK1Q ask1, Database *database)
{
	GNode *indi = ask_for_indi(ttl, ask1, database);
	if (!indi) return NULL;
	GNode *node = nztop(indi);
	releaseRecord(indi);
	return nxref(node);
}
/*===============================================================
 * chooseOneFromSequenceIfNeeded  -- handle ask1 cases
 *  seq:   [IN]  sequence from which to choose
 *  ask1:  [IN]  whether to prompt if only one element in sequence
 *  titl1: [IN]  title if sequence has one element
 *  titln: [IN]  title if sequence has multiple elements
 *=============================================================*/
static int
chooseOneFromSequenceIfNeeded (Sequence *seq, ASK1Q ask1, CString titl1,
			       CString titln, enum SequenceType type)
{
	if (lengthSequence(seq) > 1)
		return chooseOneFromSequence(titln, seq, type);
	else if (ask1==DOASK1 && titl1)
		return chooseOneFromSequence(titl1, seq, type);
	return 0;
}
/*======================================================
 * chooseFromSequence -- Format sequence and have user
 *  choose from it (any type)
 * This handles bad pointers, which can get into the data
 *  several ways.
 *  seq:   [IN]  sequence from which to choose
 *  ask1:  [IN]  whether to prompt if only one element in sequence
 *  titl1: [IN]  title if sequence has one element
 *  titln: [IN]  title if sequence has multiple elements
 *=====================================================*/
GNode *
chooseFromSequence (Sequence *seq, ASK1Q ask1, CString titl1, CString titln,
		    enum SequenceType type)
{
	int i = 0;
	GNode *rec=0;

	i = chooseOneFromSequenceIfNeeded(seq, ask1, titl1, titln, type);
	if (i == -1) return NULL;
	CString skey;

	if (! elementFromSequence (seq, i, &skey, NULL))
	  return NULL;

	rec =getRecord (skey, seq->index);

	if(!rec) {
		char buf[132];
		destrncpyf(buf, sizeof(buf), uu8, "%s", _(qSbadkeyptr));
		msg_error("%s", buf);
	}
	return rec;
}
/*===============================================
 * ask_for_record -- Ask user to identify record
 *  lookup by key or by refn (& handle dup refns)
 *  idstr: [IN]  question prompt
 *  letr:  [IN]  letter to possibly prepend to key (ie, I/F/S/E/X)
 *=============================================*/
GNode *
ask_for_record (CString idstr, int letr ATTRIBUTE_UNUSED, Database *database)
{
	GNode *rec;
	char answer[MAXPATHLEN];
	if (!ask_for_string(idstr, _(qSidkyrfn), answer, sizeof(answer)))
		return NULL;
	if (!answer[0]) return NULL;

	//rec = key_possible_to_record(answer, letr);
	rec = getRecord (answer, database->recordIndex);
	if (!rec) {
		Sequence *seq;
		seq = refnToSequence(answer, database->recordIndex,
				     database->refnIndex);
		if (!seq) return NULL;
		rec = chooseFromSequence(seq, NOASK1, _(qSduprfn), _(qSduprfn), chooseTypeDefault);
		deleteSequence(seq);
	}
	return rec;
}
/*===============================================
 * ask_for_record_key -- Ask user to enter record key
 * returns NULL or strsave'd answer
 *=============================================*/
String
ask_for_record_key (CString title, CString prompt)
{
	char answer[MAXPATHLEN];
	if (!ask_for_string(title, prompt, answer, sizeof(answer)))
		return NULL;
	if (!answer[0]) return NULL;
	return strsave(answer);
}

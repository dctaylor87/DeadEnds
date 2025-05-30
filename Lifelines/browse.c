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
 * browse.c -- Implements the browse command
 * NB: Part of curses GUI version
 * Copyright(c) 1992-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 25 Aug 93
 *   2.3.6 - 01 Nov 93    3.0.0 - 24 Sep 94
 *   3.0.2 - 16 Oct 94    3.0.2 - 30 Dec 94
 *   3.0.3 - 04 May 95
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
#include "list.h"
#include "stringtable.h"
#include "options.h"

#include "zstr.h"
#include "translat.h"
#include "refnindex.h"
#include "gnode.h"
#include "xlat.h"
#include "readwrite.h"
#include "recordindex.h"
#include "rfmt.h"
#include "sequence.h"
#include "ask.h"
#include "choose.h"
#include "errors.h"
#include "feedback.h"
#include "lineage.h"
#include "llinesi.h"
#include "liflines.h"
#include "menuitem.h"
#include "messages.h"
#include "screen.h"
#include "splitjoin.h"
#include "codesets.h"
//#include "xreffile.h"
#include "xref.h"
#include "gstrings.h"
#include "de-strings.h"
#include "ll-gedcom.h"
#include "ll-sequence.h"
#include "ll-node.h"
#include "ui.h"
#include "locales.h"
#include "lloptions.h"

#include "llpy-externs.h"

/* everything in this file assumes we are dealing with the current database */
//#define database	currentDatabase

/*********************************************
 * global/exported variables
 *********************************************/

GNode *jumpnode; /* used by Ethel for direct navigation */

/*********************************************
 * external/imported variables
 *********************************************/

//extern bool traditional;

/*********************************************
 * local enums & defines
 *********************************************/

#define MAX_SPOUSES 30
struct hist;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static GNode *add_new_rec_maybe_ref(GNode *current, char ntype);
static void ask_clear_history(struct hist * histp);
static void autoadd_xref(GNode *rec, GNode *newnode);
static int browse_aux(GNode **prec1, GNode **prec2, Sequence **pseq);
static int browse_indi(GNode **prec1, GNode **prec2, Sequence **pseq);
static int browse_fam(GNode **prec1, GNode **prec2, Sequence **pseq);
static int browse_indi_modes(GNode **prec1, GNode **prec2, Sequence **pseq
	, int indimode);
static int browse_pedigree(GNode **prec1, GNode **prec2, Sequence **pseq);
static GNode *disp_chistory_list(void);
static GNode *disp_vhistory_list(void);
static int display_aux(GNode *rec, int mode, bool reuse);
static int get_hist_count(struct hist * histp);
static Sequence *get_history_list(struct hist * histp);
static GNode *goto_fam_child(GNode *frec, int childno);
static GNode *goto_indi_child(GNode *irec, int childno);
static bool handle_aux_mode_cmds(int c, int * mode);
static int handle_history_cmds(int c, GNode **prec1);
static GNode *history_back(struct hist * histp);
static GNode *do_disp_history_list(struct hist * histp);
static void history_record(GNode *rec, struct hist * histp);
static GNode *history_fwd(struct hist * histp);
static void init_hist_lists(void);
static void init_hist(struct hist * histp, int count);
#if !defined(DEADENDS)
static void load_hist_lists(void);
static void load_nkey_list(CString key, struct hist * histp);
#endif
static void prompt_add_spouse_with_candidate(GNode *fam, GNode *save);
static GNode *pick_create_new_family(GNode *current, GNode *save, String * addstrings);
static void pick_remove_spouse_from_family(GNode *frec);
#if !defined(DEADENDS)
static void save_hist_lists(void);
static void save_nkey_list(CString key, struct hist * histp);
#endif
static void setrecord(GNode ** dest, GNode ** src);
static void term_hist_lists(void);
static void term_hist(struct hist * histp);

/*********************************************
 * local variables
 *********************************************/

/*
	A history structure is a circular buffer holding
	a list. start is the earliest entry, and the 
	past_end points just beyond the latest entry.
	If start==past_end the the buffer is full.
	(If start==-1, then there are no entries.)
	list is a dynamically allocated array, with #entries==size.
	-1 <= start < size
	0 <= past_end < size
	(NB: CMD_HISTORY_FWD will go ahead to unused entries.)

	XXX NOTE: The database is not currently stored alongside the
	key.  Just the key.  This means that if the database changes,
	all entries are instantly invalid.  Currently, this is not
	noticed.  This needs to be fixed.  XXX
*/
struct hist {
	int start;
	int past_end;
	int size;
	String *list;
};
static struct hist vhist; /* records visited */
static struct hist chist; /* records changed */


/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*=========================================
 * prompt_for_browse -- prompt for browse target
 *  when only type of browse is known
 *  prec:  [OUT]  current record
 *  code:  [I/O]  current browse type
 *  pseq:  [OUT]  current sequence
 * Sets either *prec or *pseq & sets *code to appropriate browse type
 *  returns addref'd record
 *=======================================*/
static void
prompt_for_browse (GNode ** prec, int * code, Sequence ** pseq)
{
	int len, rc;
	CString key, name;

	/* verify & clear the output arguments */
	ASSERT(prec);
	ASSERT(pseq);
	*prec = 0;
	*pseq = 0;

	if (*code == BROWSE_INDI) {
		/* ctype of 'B' means any type but check persons first */
		*pseq = ask_for_indiseq(_(qSidplst), 'B', &rc, currentDatabase);
		if (!*pseq) return;
		if ((len = lengthSequence(*pseq)) < 1) return;
		if (len == 1) {
			elementFromSequence(*pseq, 0, &key, &name);
			*prec = keyToPerson(key, (*pseq)->index);
			deleteSequence(*pseq);
			*pseq = NULL;
			*code = BROWSE_UNK; /* not sure what we got above */
		} else {
			*code = BROWSE_LIST;
		}
		return;
	}
	if (*code == BROWSE_EVEN) {
		*prec = chooseAnyEvent();
		return;
	}
	if (*code == BROWSE_SOUR) {
		*prec = chooseAnySource();
		return;
	}
	*prec = chooseAnyOther();
	return;
}
/*=========================================
 * browse -- Main loop of browse operation.
 *  rec may be NULL (then prompt)
 *=======================================*/
void
main_browse (GNode *rec1, int code)
{
	GNode *rec2=0;
	Sequence *seq = NULL;

	if (!rec1) {
		prompt_for_browse(&rec1, &code, &seq);
	}

	if (!rec1) {
		if (!seq) return;
		if (!lengthSequence(seq)) {
			deleteSequence(seq);
			return;
		}
	}

	/*
	loop here handle user browsing around through
	persons, families, references, etc, without returning
	to main menu
	*/

	while (code != BROWSE_QUIT) {
		switch (code) {
		case BROWSE_INDI:
			code = browse_indi(&rec1, &rec2, &seq); break;
		case BROWSE_FAM:
			code = browse_fam(&rec1, &rec2, &seq); break;
		case BROWSE_PED:
			code = browse_pedigree(&rec1, &rec2, &seq); break;
		case BROWSE_TAND:
			code = browse_tandem(&rec1, &rec2, &seq); break;
		case BROWSE_2FAM:
			code = browse_2fam(&rec1, &rec2, &seq); break;
		case BROWSE_LIST:
			code = browse_list(&rec1, &rec2, &seq); break;
		case BROWSE_SOUR:
		case BROWSE_EVEN:
		case BROWSE_AUX:
			code = browse_aux(&rec1, &rec2, &seq); break;
		case BROWSE_UNK:
			ASSERT(rec1);
			switch(nztype(rec1)) {
			case GRPerson: code=BROWSE_INDI; break;
			case GRFamily: code=BROWSE_FAM; break;
			case GRSource: code=BROWSE_SOUR; break;
			case GREvent: code=BROWSE_EVEN; break;
			default: code=BROWSE_AUX; break;
			}
		}
	}

	setrecord(&rec1, NULL);
	setrecord(&rec2, NULL);

	ASSERT(!seq);
}
/*================================================
 * goto_indi_child - jump to child by number
 * returns addref'd record
 *==============================================*/
static GNode *
goto_indi_child (GNode *irec, int childno)
{
  //int num1;
  GNode *answer = 0;
  CString akey=0; /* answer key */
  GNode *indi = nztop(irec);
  if (!irec)
    return NULL;
  FORFAMS(indi, fam, num1, currentDatabase)
    FORCHILDREN(fam, chil, key, num2, currentDatabase->recordIndex)
      if (num2 == childno) 
        akey = key;
    ENDCHILDREN
  ENDFAMS
  if (akey) {
    answer = keyToPersonRecord(akey, currentDatabase);
    addrefRecord(answer);
  }
  return answer;
}

/*================================================
 * goto_fam_child - jump to child by number
 * returns addref'd record
 *==============================================*/
static GNode *
goto_fam_child (GNode *frec, int childno)
{
  GNode *answer = 0;
  CString akey=0;
  GNode *fam = nztop(frec);
  if (!frec)
    return NULL;
  FORCHILDREN(fam, chil, key, num, currentDatabase->recordIndex)
    if (num == childno) 
      akey = key;
  ENDCHILDREN
  if (akey) {
    answer = keyToPersonRecord(akey, currentDatabase);
    addrefRecord(answer);
  }
  return answer;
}
/*===============================================
 * pick_create_new_family -- 
 * returns addref'd record
 *=============================================*/
static GNode *
pick_create_new_family (GNode *current, GNode *save, String * addstrings)
{
	int i;
	GNode *rec=0;

	i = chooseFromArray(_(qSidfcop), 2, addstrings);
	if (i == -1) return NULL;
	if (i == 0) {
		rec = add_family_by_edit(NULL, NULL, current, false);
	} else if (save) {
		char scratch[100];
		String name = personToName(nztop(save), 55);
		destrncpyf(scratch, sizeof(scratch), uu8, "%s%s", _(qSissnew), name);
		if (keyflag) {
			String key = nxref(nztop(save))+1;
			destrappf(scratch, sizeof(scratch), uu8, " (%s)", key);
		}
		if (ask_yes_or_no(scratch))
			rec = add_family_by_edit(current, save, NULL, false);
		else
			rec = add_family_by_edit(current, NULL, NULL, false);
	} else
		rec = add_family_by_edit(current, NULL, NULL, false);
	return rec;
}
/*====================================================
 * setrecord -- Move record in src to dest
 *  Handles releasing old reference
 *==================================================*/
static void
setrecord (GNode ** dest, GNode ** src)
{
	ASSERT(dest);
	if (*dest) {
		releaseRecord(*dest);
	}
	if (src) {
		*dest = *src;
		*src = 0;
	} else {
		*dest = 0;
	}
}
/*====================================================
 * browse_indi_modes -- Handle person/pedigree browse.
 *  prec1 [I/O]  current record (or upper in tandem screens)
 *  prec2 [I/O]  lower record in tandem screens
 *  pseq  [I/O]  current sequence in list browse
 *==================================================*/
static int
browse_indi_modes (GNode **prec1, GNode **prec2, Sequence **pseq, int indimode)
{
	GNode *current=0;
	CString key, name;
	int i, c, rc;
	bool reuse=false; /* flag to reuse same display strings */
	int indimodep;
	CString nkeyp;
	GNode *save=0, *tmp=0, *tmp2=0;
	Sequence *seq = NULL;
	int rtn=0; /* return code */

	ASSERT(prec1);
	ASSERT(*prec1);
	ASSERT(nztype(*prec1)==GRPerson);
	ASSERT(!*prec2);
	ASSERT(!*pseq);

	/* move working record into current */
	setrecord(&current, prec1);
	setrecord(prec1, 0);
	setrecord(prec2, 0);

	show_reset_scroll();
	nkeyp = 0;
	indimodep = indimode;

	while (true) {
		setrecord(&tmp, NULL);
		if (! nkeyp || nestr (nzkey(current), nkeyp) ||
		    (indimode != indimodep)) {
			show_reset_scroll();
		}
		history_record(current, &vhist);
			/* display & get input */
		display_indi(current, indimode, reuse);
		c = interact_indi();
		/* last keynum & mode, so can tell if changed */
		nkeyp = nzkey(current);
		indimodep = indimode;
reprocess_indi_cmd: /* so one command can forward to another */
		reuse = false; /* don't reuse display unless specifically set */
		if (c != CMD_NEWFAMILY) save = NULL;
		if (handle_menu_cmds(c, &reuse))
			continue;
		if (handle_scroll_cmds(c, &reuse))
			continue;
		if (handle_indi_mode_cmds(c, &indimode))
			continue;
		i = handle_history_cmds(c, prec1);
		if (i == 1)
			continue; /* history cmd handled, stay here */
		if (i == -1) {
			/* history cmd handled, leave page */
			rtn = BROWSE_UNK;
			goto exitbrowse;
		}
		switch (c)
		{
		case CMD_EDIT:	/* Edit this person */
			edit_indi(current, false);
			break;
		case CMD_FAMILY: 	/* Browse to person's family */
			if ((tmp = chooseFamily(current, _(qSntprnt),
						_(qSidfbrs), true, currentDatabase))) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_FAM;
				goto exitbrowse;
			}
			break;
		case CMD_TANDEM_FAMILIES:
			if ((tmp = chooseFamily(current, _(qSntprnt),
						_(qSid1fbr), true, currentDatabase)) != 0) {
				if ((tmp2 = chooseFamily(current, _(qSntprnt),
					_(qSid2fbr), true, currentDatabase)) != 0) {
					setrecord(prec1, &tmp);
					setrecord(prec2, &tmp2);
					rtn = BROWSE_2FAM;
					goto exitbrowse;
				}
				setrecord(&tmp, 0);
			}
			break;
		case CMD_FATHER: 	/* Browse to person's father */
		  if ((tmp = chooseFather(current, NULL, _(qSnofath),
				_(qSidhbrs), NOASK1, currentDatabase)) != 0) {
				setrecord(&current, &tmp);
			}
			break;
		case CMD_TANDEM_FATHERS:	/* Tandem Browse to person's fathers */
			if ((tmp = chooseFather(current, NULL, _(qSnofath),
						_(qSid1hbr), NOASK1, currentDatabase)) != 0) {
				if ((tmp2 = chooseFather(current, NULL, _(qSnofath),
							 _(qSid2hbr), NOASK1, currentDatabase)) != 0) {
					setrecord(prec1, &tmp);
					setrecord(prec2, &tmp2);
					rtn = BROWSE_TAND;
					goto exitbrowse;
				}
				setrecord(&tmp, 0);
			}
			break;
		case CMD_MOTHER:	/* Browse to person's mother */
			if ((tmp = chooseMother(current, NULL, _(qSnomoth),
						_(qSidwbrs), NOASK1, currentDatabase)) != 0) {
				setrecord(&current, &tmp);
			}
			break;
		case CMD_TANDEM_MOTHERS:	/* Tandem Browse to person's mothers */
			if ((tmp = chooseMother(current, NULL, _(qSnomoth),
				_(qSid1wbr), NOASK1, currentDatabase)) != 0) {
				if ((tmp2 = chooseMother(current, NULL, _(qSnomoth),
							 _(qSid2wbr), NOASK1, currentDatabase)) != 0) {
					setrecord(prec1, &tmp);
					setrecord(prec2, &tmp2);
					rtn = BROWSE_TAND;
					goto exitbrowse;
				}
				setrecord(&tmp, 0);
			}
			break;
		case CMD_BROWSE_ZIP_INDI:	/* Zip browse another person */
			if ((tmp = ask_for_indi(_(qSidpnxt), NOASK1, currentDatabase)) != 0)
				setrecord(&current, &tmp);
			break;
		case CMD_BROWSE_ZIP_ANY:	/* Zip browse any record */
			if ((tmp = ask_for_any(_(qSidnxt), NOASK1, currentDatabase)) != 0) {
				if (nztype(tmp) != GRPerson) {
					setrecord(prec1, &tmp);
					rtn = BROWSE_UNK;
					goto exitbrowse;
				} else {
					setrecord(&current, &tmp);
				}
			}
			break;
		case CMD_SPOUSE:	/* Browse to person's spouse */
			if ((tmp = chooseSpouse(current, _(qSnospse), _(qSidsbrs), currentDatabase)) != 0)
				setrecord(&current, &tmp);
			break;
		case CMD_TANDEM_SPOUSES:	/* browse to tandem spouses */
			if ((tmp = chooseSpouse(current, _(qSnospse), _(qSid1sbr), currentDatabase)) != 0) {
				if ((tmp2 = chooseSpouse(current, _(qSnospse), _(qSid2sbr), currentDatabase)) != 0) {
					setrecord(prec1, &tmp);
					setrecord(prec2, &tmp2);
					rtn = BROWSE_TAND;
					goto exitbrowse;
				}
				setrecord(&tmp, 0);
			}
			break;
		case CMD_CHILDREN:	/* Browse to person's child */
			if ((tmp = chooseChild(current, NULL, _(qSnocofp),
				_(qSidcbrs), NOASK1, currentDatabase)) != 0) {
				setrecord(&current, &tmp);
			}
			break;
		case CMD_TOGGLE_PEDTYPE:       /* toggle pedigree mode (ancestors/descendants) */
			pedigree_toggle_mode();
			break;
		case CMD_DEPTH_DOWN:       /* decrease pedigree depth */
			pedigree_increase_generations(-1);
			break;
		case CMD_DEPTH_UP:      /* increase pedigree depth */
			pedigree_increase_generations(+1);
			break;
		case CMD_TOGGLE_CHILDNUMS:       /* toggle children numbers */
			show_childnumbers();
			break;
		case CMD_CHILD_DIRECT0+1:	/* Go to children by number */
		case CMD_CHILD_DIRECT0+2:
		case CMD_CHILD_DIRECT0+3:
		case CMD_CHILD_DIRECT0+4:
		case CMD_CHILD_DIRECT0+5:
		case CMD_CHILD_DIRECT0+6:
		case CMD_CHILD_DIRECT0+7:
		case CMD_CHILD_DIRECT0+8:
		case CMD_CHILD_DIRECT0+9:
			if ((tmp = goto_indi_child(current, c-CMD_CHILD_DIRECT0)) != 0)
				setrecord(&current, &tmp);
			else
				message("%s", _(qSnochil));
			break;
		case CMD_TANDEM_CHILDREN:	/* browse to tandem children */
			if ((tmp = chooseChild(current, NULL, _(qSnocofp),
					       _(qSid1cbr), NOASK1, currentDatabase)) != 0) {
				if ((tmp2 = chooseChild(current, NULL, _(qSnocofp),
					_(qSid2cbr), NOASK1, currentDatabase)) != 0) {
					setrecord(prec1, &tmp);
					setrecord(prec2, &tmp2);
					rtn = BROWSE_TAND;
					goto exitbrowse;
				}
				setrecord(&tmp, 0);
			}
			break;
		case CMD_PEDIGREE:	/* Switch to pedigree mode */
			if (indimode == 'i')
				indimode = 'p';
			else
				indimode = 'i';
			break;
		case CMD_UPSIB:	/* Browse to older sib */
			if ((tmp = indi_to_prev_sib(current, currentDatabase)) != 0)
				setrecord(&current, &tmp);
			else
				msg_error("%s", _(qSnoosib));
			break;
		case CMD_DOWNSIB:	/* Browse to younger sib */
			if ((tmp = indi_to_next_sib(current, currentDatabase)) != 0)
				setrecord(&current, &tmp);
			else
				msg_error("%s", _(qSnoysib));
			break;
		case CMD_PARENTS:	/* Browse to parents' family */
			if ((tmp = chooseFamily(current, _(qSnoprnt),
				_(qSidfbrs), false, currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_FAM;
				goto exitbrowse;
			}
			break;
		case CMD_TANDEM_PARENTS:	/* tandem browse to two parents families*/
			if ((tmp = chooseFamily(current, _(qSnoprnt),
						_(qSid1fbr), false, currentDatabase)) != 0) {
				if ((tmp2 = chooseFamily(current, _(qSnoprnt),
					_(qSid2fbr), false, currentDatabase)) != 0) {
					setrecord(prec1, &tmp);
					setrecord(prec2, &tmp2);
					rtn = BROWSE_2FAM;
					goto exitbrowse;
				}
				setrecord(&tmp, 0);
			}
			break;
		case CMD_BROWSE: 	/* Browse new list of persons */
			seq = ask_for_indiseq(_(qSidplst), 'B', &rc, currentDatabase);
			if (!seq) break;
			if (lengthSequence(seq) == 1) {
				elementFromSequence(seq, 0, &key, &name);
				tmp = keyToPerson(key, seq->index);
				setrecord(&current, &tmp);
				deleteSequence(seq);
				seq=NULL;
				if (nztype(current) != GRPerson) {
					setrecord(prec1, &current);
					rtn = BROWSE_UNK;
					goto exitbrowse;
				}
			} else {
				*pseq = seq;
				rtn = BROWSE_LIST;
				goto exitbrowse;
			}
			break;
		case CMD_NEWPERSON:	/* Add new person */
			if (!(tmp = add_indi_by_edit(false)))
				break;
			setrecord(&save, &current);
			setrecord(&current, &tmp);
			break;
		case CMD_NEWFAMILY:	/* Add family for current person */
			{
				String addstrings[2];
				addstrings[0] = _(qScrtcfm);
				addstrings[1] = _(qScrtsfm);
				if ((tmp = pick_create_new_family(current, save, addstrings)) != 0) {
					setrecord(&save, NULL);
					setrecord(prec1, &tmp);
					rtn = BROWSE_FAM;
					goto exitbrowse;
				}
			}
			break;
		case CMD_ADD_SOUR: /* add source */
		case CMD_ADD_EVEN: /* add event */
		case CMD_ADD_OTHR: /* add other */
			{
				char c2 = (c==CMD_ADD_SOUR ? 'S' : (c==CMD_ADD_EVEN ? 'E' : 'X'));
				if ((tmp = add_new_rec_maybe_ref(current, c2)) != 0) {
					if (tmp == current) {
						c = CMD_EDIT;
						goto reprocess_indi_cmd; /* forward to edit */
					}
					setrecord(prec1, &tmp);
					rtn = BROWSE_UNK;
					goto exitbrowse;
				}
			}
			break;
		case CMD_TANDEM:	/* Switch to tandem browsing */
			if ((tmp = ask_for_indi(_(qSidp2br), NOASK1, currentDatabase)) != 0) {
				setrecord(prec1, &current);
				setrecord(prec2, &tmp);
				rtn = BROWSE_TAND;
				goto exitbrowse;
			}
			break;
		case CMD_SWAPFAMILIES: 	/* Swap families of current person */
			swap_families(current);
			break;
		case CMD_ADDASSPOUSE:	/* Add person as spouse */
			prompt_add_spouse(current, NULL, true);
			break;
		case CMD_ADDASCHILD:    /* Add person as child */
			my_prompt_add_child(nztop(current), NULL);
			break;
		case CMD_PERSON:   /* switch to person browse */
			indimode='i';
			break;
		case CMD_REMOVEASSPOUSE:	/* Remove person as spouse */
			chooseAndRemoveSpouse(current, NULL, false);
			break;
		case CMD_REMOVEASCHILD:	/* Remove person as child */
			chooseAndRemoveChild(current, NULL, false);
			break;
		case CMD_ADVANCED:	/* Advanced person edit */
			advanced_person_edit(nztop(current));
			break;
		case CMD_JUMP_HOOK:   /* GUI direct navigation */
			current = jumpnode;
			break;
		case CMD_NEXT:	/* Go to next indi in db */
			{
				tmp = getNextPersonRecord (nkeyp, currentDatabase);
				if (tmp)
					setrecord (&current, &tmp);
				else
					msg_error ("%s", _(qSnopers));
			}
			break;
		case CMD_PREV:	/* Go to prev indi in db */
			{
			  tmp = getPreviousPersonRecord (nkeyp, currentDatabase);
				if (tmp)
					setrecord (&current, &tmp);
				else
					msg_error ("%s", _(qSnopers));
			}
			break;
		case CMD_SOURCES:	/* Browse to sources */
			if ((tmp = chooseSource(current, _(qSnosour), _(qSidsour), currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_AUX;
				goto exitbrowse;
			}
			break;
		case CMD_NOTES:	/* Browse to notes */
			if ((tmp = chooseNote(current, _(qSnonote), _(qSidnote), currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_AUX;
				goto exitbrowse;
			}
			break;
		case CMD_POINTERS:	/* Browse to references */
			if ((tmp = choosePointer(current, _(qSnoptr), _(qSidptr), currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_UNK;
				goto exitbrowse;
			}
			break;
		case CMD_QUIT:
			rtn = BROWSE_QUIT;
			goto exitbrowse;
		}
	}
exitbrowse:
	setrecord(&tmp, NULL);
	setrecord(&current, NULL);
	return rtn;
}
/*==========================================
 * display_aux -- Show aux node in current mode
 * Created: 2001/01/27, Perry Rapp
 *========================================*/
static int
display_aux (GNode *rec, int mode, bool reuse)
{
	int c;
	c = aux_browse(rec, mode, reuse);
	return c;
}
/*====================================================
 * browse_aux -- Handle aux node browse.
 * Created: 2001/01/27, Perry Rapp
 *==================================================*/
static int
browse_aux (GNode **prec1, GNode **prec2, Sequence **pseq)
{
	GNode *current=0;
	int i, c;
	bool reuse=false; /* flag to reuse same display strings */
	int auxmode=0, auxmodep=0;
	CString nkeyp=0;
	RecordType ntype=GRUnknown, ntypep=GRUnknown;
	GNode *tmp=0;
	char c2;
	int rtn=0; /* return code */

	ASSERT(prec1);
	ASSERT(*prec1);
	ASSERT(!*prec2);
	ASSERT(!*pseq);

	/* move working record into current */
	setrecord(&current, prec1);
	setrecord(prec1, 0);
	setrecord(prec2, 0);


	auxmode = 'x';

	show_reset_scroll();
	nkeyp = 0;
	ntypep = 0;
	auxmodep = auxmode;

	while (true) {
		if (! nkeyp ||nestr(nzkey(current), nkeyp) ||
		    (nztype(current) != ntypep) ||
		    (auxmode != auxmodep)) {
			show_reset_scroll();
		}
		ntype = nztype(current);
		history_record(current, &vhist);
		c = display_aux(current, auxmode, reuse);
		/* last keynum & mode, so can tell if changed */
		nkeyp = nzkey(current);
		ntypep = nztype(current);
		auxmodep = auxmode;
reprocess_aux_cmd:
		reuse = false; /* don't reuse display unless specifically set */
		if (handle_menu_cmds(c, &reuse))
			continue;
		if (handle_scroll_cmds(c, &reuse))
			continue;
		if (handle_aux_mode_cmds(c, &auxmode))
			continue;
		i = handle_history_cmds(c, prec1);
		if (i == 1)
			continue; /* history cmd handled, stay here */
		if (i == -1)
			return BROWSE_UNK; /* history cmd handled, leave page */
		switch (c)
		{
		case CMD_EDIT:
			switch(ntype) {
			case GRSource: edit_source(current, false); break;
			case GREvent: edit_event(current, false); break;
			case GROther: edit_other(current, false); break;
			default:
				break;
			}
			break;
		case CMD_ADD_SOUR: /* add source */
		case CMD_ADD_EVEN: /* add event */
		case CMD_ADD_OTHR: /* add other */
			c2 = (c==CMD_ADD_SOUR ? 'S' : (c==CMD_ADD_EVEN ? 'E' : 'X'));
			if ((tmp = add_new_rec_maybe_ref(current, c2)) != 0) {
				if (tmp == current) {
					c = CMD_EDIT;
					goto reprocess_aux_cmd; /* forward to edit */
				} else {
					setrecord(prec1, &tmp);
					rtn = BROWSE_UNK;
					goto exitbrowse;
				}
			}
			break;
		case CMD_BROWSE_ZIP_INDI:	/* Zip browse to new person */
			if ((tmp = ask_for_indi(_(qSidpnxt), NOASK1, currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_UNK;
				goto exitbrowse;
			}
			break;
		case CMD_BROWSE_ZIP_ANY:	/* Zip browse any record */
			if ((tmp = ask_for_any(_(qSidnxt), NOASK1, currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_UNK;
				goto exitbrowse;
			}
			break;
		case CMD_NOTES:	/* Browse to notes */
			if ((tmp = chooseNote(current, _(qSnonote), _(qSidnote), currentDatabase)) != 0) {
				setrecord(&current, &tmp);
			}
			break;
		case CMD_POINTERS:	/* Browse to references */
			if ((tmp = choosePointer(current, _(qSnoptr), _(qSidptr), currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_UNK;
				goto exitbrowse;
			}
			break;
		case CMD_NEXT:	/* Go to next in db */
			{
				if (! nkeyp)
					tmp = getFirstRecord (ntype, currentDatabase);
				else
					tmp = getNextRecord (ntype, nkeyp, currentDatabase);
				break;
			}
		case CMD_PREV:	/* Go to prev in db */
			{
				if (! nkeyp)
					tmp = getLastRecord (ntype, currentDatabase);
				else
					tmp = getPreviousRecord (ntype, nkeyp, currentDatabase);
				break;
			}
		case CMD_QUIT:
			rtn = BROWSE_QUIT;
			goto exitbrowse;
		}
	}
exitbrowse:
	setrecord(&current, NULL);
	return rtn;
}
/*================================================
 * browse_indi -- Handle person browse operations.
 *==============================================*/
static int
browse_indi (GNode **prec1, GNode **prec2, Sequence **pseq)
{
	return browse_indi_modes(prec1, prec2, pseq, 'n');
}
/*===============================================
 * pick_remove_spouse_from_family -- 
 *  pulled out of browse_fam, 2001/02/03, Perry Rapp
 *  construct list of spouses, prompt for choice, & remove
 *=============================================*/
static void
pick_remove_spouse_from_family (GNode *frec)
{
	GNode *fam = nztop(frec);
	GNode *fref, *husb, *wife, *chil, *rest;
	GNode *root, *node, *spnodes[MAX_SPOUSES];
	String spstrings[MAX_SPOUSES];
	int i;

	splitFamily(fam, &fref, &husb, &wife, &chil, &rest);
	if (!husb && !wife) {
		msg_error("%s", _(qShasnei));
		return;
	}
	i = 0;
	for (node = husb; node; node = nsibling(node)) {
		root = keyToPerson(nval(node), currentDatabase->recordIndex);
		spstrings[i] = indi_to_list_string(root,
			 NULL, 66, true, true);
		spnodes[i++] = root;
	}
	for (node = wife; node; node = nsibling(node)) {
		root = keyToPerson(nval(node), currentDatabase->recordIndex);
		spstrings[i] = indi_to_list_string(root,
			 NULL, 66, true, true);
		spnodes[i++] = root;
		if (i == MAX_SPOUSES) {
			message("%s", _(qSspover));
			break;
		}
	}
	joinFamily(fam, fref, husb, wife, chil, rest);
	i = chooseFromArray(_(qSidsrmv), i, spstrings);
	if (i == -1) return;
#if defined(DEADENDS)
	node = spnodes[i];
	while (node->parent)
	  node = node->parent;

	chooseAndRemoveSpouse(node, frec, true);
#else
	chooseAndRemoveSpouse(_llpy_node_to_record(spnodes[i], currentDatabase), frec, true);
#endif
}
/*===============================================
 * prompt_add_spouse_with_candidate -- 
 *  fam:  [IN]  family to which to add (required arg)
 *  save: [IN]  candidate spouse to add (optional arg)
 * If candidate passed, asks user if that is desired spouse to add
 * In either case, all work is delegated to prompt_add_spouse
 *=============================================*/
static void
prompt_add_spouse_with_candidate (GNode *fam, GNode *candidate)
{
	GNode *fref, *husb, *wife, *chil, *rest;
	bool confirm;
	char scratch[100];

	splitFamily(nztop(fam), &fref, &husb, &wife, &chil, &rest);
	joinFamily(nztop(fam), fref, husb, wife, chil, rest);
	if (traditional) {
		if (husb && wife) {
			msg_error("%s", _(qShasbth));
			return;
		}
	}
	if (candidate) {
		if (keyflag) {
			snprintf(scratch, sizeof(scratch), "%s%s (%s)", _(qSissnew),
				 personToName(nztop(candidate), 56),
				 nxref(nztop(candidate))+1);
		} else {
			snprintf(scratch, sizeof(scratch), "%s%s", _(qSissnew),
				 personToName(nztop(candidate), 56));
		}
		if (!ask_yes_or_no(scratch)) {
			candidate = NULL;
		}
	}
		/* don't confirm again if they just confirmed candidate */
	confirm = (candidate == NULL); 
	prompt_add_spouse(candidate, fam, confirm);;
}
/*===============================================
 * prompt_add_child_check_save -- 
 *  fam:  [IN]  family to which to add (required arg)
 *  save: [IN]  possible child to add (optional arg)
 * If save is passed, this checks with user whether that is desired child
 * In either case, all work is delegated to prompt_add_child
 *=============================================*/
static void
prompt_add_child_check_save (GNode *fam, GNode *save)
{
	char scratch[100];

	if (save) {
		if (keyflag)
			if(getdeoptint("DisplayKeyTags", 0) > 0) {
				snprintf(scratch, sizeof(scratch), "%s%s (i%s)", _(qSiscnew),
				 	personToName(save, 56),
				 	nxref(save)+1);
			} else {
				snprintf(scratch, sizeof(scratch), "%s%s (%s)", _(qSiscnew),
				 	personToName(save, 56),
				 	nxref(save)+1);
			}
		else
			snprintf(scratch, sizeof(scratch), "%s%s", _(qSiscnew),
				 personToName(save, 56));
		if (!ask_yes_or_no(scratch))
			save = NULL;
	}
	my_prompt_add_child(save, fam);
}
/*===============================================
 * my_prompt_add_child -- call prompt_add_child with our reformatting info
 *=============================================*/
GNode *
my_prompt_add_child (GNode *child, GNode *fam)
{
	return prompt_add_child(child, fam, true);
}
/*===============================================
 * browse_fam -- Handle family browse selections.
 *=============================================*/
static int
browse_fam (GNode **prec1, GNode **prec2, Sequence **pseq)
{
	GNode *current=0;
	int i, c, rc;
	bool reuse=false; /* flag to reuse same display strings */
	static int fammode='n';
	int fammodep;
	CString nkeyp;
	GNode *save=0, *tmp=0, *tmp2=0;
	Sequence *seq;
	CString key, name;
	char c2;
	int rtn=0; /* return code */

	ASSERT(prec1);
	ASSERT(*prec1);
	ASSERT(nztype(*prec1)==GRFamily);
	ASSERT(!*prec2);
	ASSERT(!*pseq);

	/* move working record into current */
	setrecord(&current, prec1);
	setrecord(prec1, 0);
	setrecord(prec2, 0);

	show_reset_scroll();
	nkeyp = 0;
	fammodep = fammode;

	while (true) {
		setrecord(&tmp, NULL);
		if (! nkeyp || nestr (nzkey(current), nkeyp) ||
		    (fammode != fammodep)) {
			show_reset_scroll();
		}
		history_record(current, &vhist);
		display_fam(current, fammode, reuse);
		c = interact_fam();
		/* last keynum & mode, so can tell if changed */
		nkeyp = nzkey(current);
		fammodep = fammode;
reprocess_fam_cmd: /* so one command can forward to another */
		reuse = false; /* don't reuse display unless specifically set */
		if (c != CMD_ADDCHILD && c != CMD_ADDSPOUSE)
			save = NULL;
		if (handle_menu_cmds(c, &reuse))
			continue;
		if (handle_scroll_cmds(c, &reuse))
			continue;
		if (handle_fam_mode_cmds(c, &fammode))
			continue;
		i = handle_history_cmds(c, prec1);
		if (i == 1)
			continue; /* history cmd handled, stay here */
		if (i == -1) {
			/* history cmd handled, leave page */
			rtn = BROWSE_UNK;
			goto exitbrowse;
		}
		switch (c) 
		{
		case CMD_ADVANCED:	/* Advanced family edit */
			advanced_family_edit(nztop(current));
			break;
		case CMD_BROWSE_FAM:
		{
			char buffer[100];
			if (ask_for_string (_(qSidfamk), _(qSaskstr),  buffer, sizeof (buffer)))
			{
				tmp = keyToFamilyRecord (buffer, currentDatabase);
				if (tmp)
					setrecord (&current, &tmp);
				else
					msg_error ("%s", _(qSnofam));
			}
		}
			break;
		case CMD_EDIT:	/* Edit family's record */
			edit_family(current, false);
			break;
		case CMD_FATHER:	/* Browse to family's father */
			if ((tmp = chooseFather(NULL, current, _(qSnohusb),
				_(qSidhbrs), NOASK1, currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_INDI;
				goto exitbrowse;
			}
			break;
		case CMD_TANDEM_FATHERS:	/* Tandem Browse to family's fathers */
			if ((tmp = chooseFather(NULL, current, _(qSnohusb),
				_(qSid1hbr), NOASK1, currentDatabase)) != 0) {
				if ((tmp2 = chooseFather(NULL, current, _(qSnohusb),
					_(qSid2hbr), NOASK1, currentDatabase)) != 0) {
					setrecord(prec1, &tmp);
					setrecord(prec2, &tmp2);
					rtn = BROWSE_TAND;
					goto exitbrowse;
				}
				setrecord(&tmp, 0);
			}
			break;
		case CMD_MOTHER:	/* Browse to family's mother */
			if ((tmp = chooseMother(NULL, current, _(qSnowife),
				_(qSidwbrs), NOASK1, currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_INDI;
				goto exitbrowse;
			}
			break;
		case CMD_TANDEM_MOTHERS:	/* Tandem Browse to family's mother */
			if ((tmp = chooseMother(NULL, current, _(qSnowife),
				_(qSid1wbr), NOASK1, currentDatabase)) != 0) {
				if ((tmp2 = chooseMother(NULL, current, _(qSnowife), 
					_(qSid2wbr), NOASK1, currentDatabase)) != 0) {
					setrecord(prec1, &tmp);
					setrecord(prec2, &tmp2);
					rtn = BROWSE_TAND;
					goto exitbrowse;
				}
				setrecord(&tmp, 0);
			}
			break;
		case CMD_CHILDREN:	/* Browse to a child */
			if ((tmp = chooseChild(NULL, current, _(qSnocinf),
					       _(qSidcbrs), NOASK1, currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_INDI;
				goto exitbrowse;
			}
			break;
		case CMD_TANDEM_CHILDREN:	/* browse to tandem children */
			if ((tmp = chooseChild(NULL, current, _(qSnocinf),
					       _(qSid1cbr), NOASK1, currentDatabase)) != 0) {
				if ((tmp2 = chooseChild(NULL, current, _(qSnocinf),
					_(qSid2cbr), NOASK1, currentDatabase)) != 0) {
					setrecord(prec1, &tmp);
					setrecord(prec2, &tmp2);
					rtn = BROWSE_TAND;
					goto exitbrowse;
				}
				setrecord(&tmp, 0);
			}
			break;
		case CMD_REMOVECHILD:	/* Remove a child */
			if ((tmp = chooseChild(NULL, current, _(qSnocinf),
				_(qSidcrmv), DOASK1, currentDatabase)) != 0) {
				chooseAndRemoveChild(tmp, current, true);
				setrecord(&tmp, 0);
			}
			break;
		case CMD_ADDSPOUSE:	/* Add spouse to family */
			prompt_add_spouse_with_candidate(current, save);
			setrecord(&save, 0);
			break;
		case CMD_REMOVESPOUSE:	/* Remove spouse from family */
			pick_remove_spouse_from_family(current);
			break;
		case CMD_NEWPERSON:	/* Add person to database */
			tmp = add_indi_by_edit(false);
			setrecord(&save, &tmp);
			break;
		case CMD_ADD_SOUR: /* add source */
		case CMD_ADD_EVEN: /* add event */
		case CMD_ADD_OTHR: /* add other */
			c2 = (c==CMD_ADD_SOUR ? 'S' : (c==CMD_ADD_EVEN ? 'E' : 'X'));
			if ((tmp = add_new_rec_maybe_ref(current, c2)) != 0) {
				if (tmp == current) {
					c = CMD_EDIT;
					goto reprocess_fam_cmd; /* forward to edit */
				} else {
					setrecord(prec1, &tmp);
					rtn = BROWSE_UNK;
					goto exitbrowse;
				}
			}
			break;
		case CMD_ADDCHILD:	/* Add child to family */
			prompt_add_child_check_save(nztop(current), nztop(save));
			setrecord(&save, 0);
			break;
		case CMD_BROWSE: 	/* Browse to new list of persons */
			seq = ask_for_indiseq(_(qSidplst), 'B', &rc, currentDatabase);
			if (!seq) break;
			if (lengthSequence(seq) == 1) {
				elementFromSequence(seq, 0, &key, &name);
				tmp = keyToPerson(key, seq->index);
				setrecord(&current, &tmp);
				deleteSequence(seq);
				seq=NULL;
				if (nztype(current) != GRFamily) {
					setrecord(prec1, &current);
					rtn = BROWSE_UNK;
					goto exitbrowse;
				}
				break;
			}
			*pseq = seq;
			rtn = BROWSE_LIST;
			goto exitbrowse;
			break;
		case CMD_BROWSE_ZIP_INDI:	/* Zip browse to new person */
			if ((tmp = ask_for_indi(_(qSidpnxt), NOASK1, currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_INDI;
				goto exitbrowse;
			}
			break;
		case CMD_BROWSE_ZIP_ANY:	/* Zip browse any record */
			if ((tmp = ask_for_any(_(qSidnxt), NOASK1, currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_UNK;
				goto exitbrowse;
			}
			break;
		case CMD_TANDEM:	/* Enter family tandem mode */
		  if ((tmp = ask_for_fam(_(qSids2fm), _(qSidc2fm), currentDatabase)) != 0) {
				setrecord(prec1, &current);
				setrecord(prec2, &tmp);
				rtn = BROWSE_2FAM;
				goto exitbrowse;
			}
			break;
		case CMD_SWAPCHILDREN:	/* Swap two children */
			swap_children(NULL, current);
			break;
		case CMD_REORDERCHILD: /* Move a child in order */
			reorder_child(NULL, current, true);
			break;
		case CMD_TOGGLE_CHILDNUMS:       /* toggle children numbers */
			show_childnumbers();
			break;
		case CMD_CHILD_DIRECT0+1:	/* Go to children by number */
		case CMD_CHILD_DIRECT0+2:
		case CMD_CHILD_DIRECT0+3:
		case CMD_CHILD_DIRECT0+4:
		case CMD_CHILD_DIRECT0+5:
		case CMD_CHILD_DIRECT0+6:
		case CMD_CHILD_DIRECT0+7:
		case CMD_CHILD_DIRECT0+8:
		case CMD_CHILD_DIRECT0+9:
			if ((tmp = goto_fam_child(current, c-CMD_CHILD_DIRECT0)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_INDI;
				goto exitbrowse;
			}
			msg_error("%s", _(qSnochil));
			break;
		case CMD_NEXT:	/* Go to next fam in db */
			{
				tmp = getNextFamilyRecord (nkeyp, currentDatabase);
				if (tmp)
					setrecord (&current, &tmp);
				else
					msg_error ("%s", _(qSnofam));
				break;
			}
		case CMD_PREV:	/* Go to prev fam in db */
			{
			  tmp = getPreviousFamilyRecord (nkeyp, currentDatabase);
				if (tmp)
					setrecord (&current, &tmp);
				else
					msg_error ("%s", _(qSnofam));
				break;
			}
		case CMD_SOURCES:	/* Browse to sources */
			if ((tmp = chooseSource(current, _(qSnosour), _(qSidsour), currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_AUX;
				goto exitbrowse;
			}
			break;
		case CMD_NOTES:	/* Browse to notes */
			if ((tmp = chooseNote(current, _(qSnonote), _(qSidnote), currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_AUX;
				goto exitbrowse;
			}
			break;
		case CMD_POINTERS:	/* Browse to references */
			if ((tmp = choosePointer(current, _(qSnoptr), _(qSidptr), currentDatabase)) != 0) {
				setrecord(prec1, &tmp);
				rtn = BROWSE_UNK;
				goto exitbrowse;
			}
			break;
		case CMD_QUIT:
			rtn = BROWSE_QUIT;
			goto exitbrowse;
		}
	}
exitbrowse:
	setrecord(&current, NULL);
	return rtn;
}
/*======================================================
 * handle_menu_cmds -- Handle menuing commands
 * That is, handle commands that are internal to the menuing 
 * system (eg, paging, changing current menu size).
 * Created: 2001/01/31, Perry Rapp
 *====================================================*/
bool
handle_menu_cmds (int c, bool * reuse)
{
	bool old = *reuse;
	/* if a menu command, then we CAN reuse the previous display strings */
	*reuse = true;
	switch(c) {
		case CMD_MENU_GROW: adjust_browse_menu_height(+1); return true;
		case CMD_MENU_SHRINK: adjust_browse_menu_height(-1); return true;
		case CMD_MENU_MORECOLS: adjust_browse_menu_cols(+1); return true;
		case CMD_MENU_LESSCOLS: adjust_browse_menu_cols(-1); return true;
		case CMD_MENU_MORE: cycle_browse_menu(); return true;
		case CMD_MENU_TOGGLE: toggle_browse_menu(); return true;
	}
	*reuse = old;
	return false;
}
/*======================================================
 * handle_scroll_cmds -- Handle detail scrolling
 * Created: 2001/02/01, Perry Rapp
 *====================================================*/
bool
handle_scroll_cmds (int c, bool * reuse)
{
	bool old = *reuse;
	/* if a menu command, then we CAN reuse the previous display strings */
	*reuse = true;
	switch(c) {
		case CMD_SCROLL_UP: show_scroll(-1); return true;
		case CMD_SCROLL_DOWN: show_scroll(+1); return true;
	}
	*reuse = old;
	return false;
}
/*======================================================
 * handle_indi_mode_cmds -- Handle indi modes
 * Created: 2001/02/04, Perry Rapp
 *====================================================*/
bool
handle_indi_mode_cmds (int c, int * mode)
{
	switch(c) {
		case CMD_MODE_GEDCOM: *mode = 'g'; return true;
		case CMD_MODE_GEDCOMX: *mode = 'x'; return true;
		case CMD_MODE_GEDCOMT: *mode = 't'; return true;
		case CMD_MODE_PEDIGREE:
			*mode = (*mode=='a')?'d':'a';
			return true;
		case CMD_MODE_ANCESTORS: *mode = 'a'; return true;
		case CMD_MODE_DESCENDANTS: *mode = 'd'; return true;
		case CMD_MODE_NORMAL: *mode = 'n'; return true;
		case CMD_MODE_CYCLE: 
			switch(*mode) {
			case 'n': *mode = 'a'; break;
			case 'a': *mode = 'd'; break;
			case 'd': *mode = 'g'; break;
			case 'g': *mode = 'x'; break;
			case 'x': *mode = 't'; break;
			case 't': *mode = 'n'; break;
			}
			return true;
	}
	return false;
}
/*======================================================
 * handle_fam_mode_cmds -- Handle indi modes
 * Created: 2001/02/04, Perry Rapp
 *====================================================*/
bool
handle_fam_mode_cmds (int c, int * mode)
{
	switch(c) {
		case CMD_MODE_GEDCOM: *mode = 'g'; return true;
		case CMD_MODE_GEDCOMX: *mode = 'x'; return true;
		case CMD_MODE_GEDCOMT: *mode = 't'; return true;
		case CMD_MODE_NORMAL: *mode = 'n'; return true;
		case CMD_MODE_CYCLE: 
			switch(*mode) {
			case 'n': *mode = 'g'; break;
			case 'g': *mode = 'x'; break;
			case 'x': *mode = 't'; break;
			case 't': *mode = 'n'; break;
			}
			return true;
	}
	return false;
}
/*======================================================
 * handle_aux_mode_cmds -- Handle aux modes
 * Created: 2001/02/11, Perry Rapp
 *====================================================*/
bool
handle_aux_mode_cmds (int c, int * mode)
{
	switch(c) {
		case CMD_MODE_GEDCOM: *mode = 'g'; return true;
		case CMD_MODE_GEDCOMX: *mode = 'x'; return true;
		case CMD_MODE_GEDCOMT: *mode = 't'; return true;
		case CMD_MODE_CYCLE: 
			switch(*mode) {
			case 'g': *mode = 'x'; break;
			case 'x': *mode = 't'; break;
			case 't': *mode = 'g'; break;
			}
			return true;
	}
	return false;
}
/*======================================================
 * browse_pedigree -- Handle pedigree browse selections.
 *====================================================*/
static int
browse_pedigree (GNode **prec1, GNode **prec2, Sequence **pseq)
{
	return browse_indi_modes(prec1, prec2, pseq, 'p');
}
/*==================================================
 * chooseAnySource -- choose from list of all sources
 *================================================*/
GNode *
chooseAnySource (void)
{
	Sequence *seq;
	GNode *rec;
	seq = getAllSources(currentDatabase);
	if (!seq)
	{
		msg_error("%s", _(qSnosour));
		return 0;
	}
	rec = chooseFromSequence(seq, DOASK1, _(qSidsour), _(qSidsour), chooseTypeDefault);
	deleteSequence(seq);
	return rec;
}
/*==================================================
 * chooseAnyEvent -- choose from list of all events
 *================================================*/
GNode *
chooseAnyEvent (void)
{
	Sequence *seq;
	GNode *rec;
	seq = getAllEvents(currentDatabase);
	if (!seq)
	{
		msg_error("%s", _(qSnoeven));
		return NULL;
	}
	rec = chooseFromSequence(seq, DOASK1, _(qSideven), _(qSideven), chooseTypeDefault);
	deleteSequence(seq);
	return rec;
}
/*==================================================
 * chooseAnyOther -- choose from list of all others
 *================================================*/
GNode *
chooseAnyOther (void)
{
	Sequence *seq;
	GNode *rec;
	seq = getAllOthers(currentDatabase);
	if (!seq)
	{
		msg_error("%s", _(qSnoothe));
		return NULL;
	}
	rec = chooseFromSequence(seq, DOASK1, _(qSidothe), _(qSidothe), chooseTypeDefault);
	deleteSequence(seq);
	return rec;
}
/*==================================================
 * init_hist_lists -- initialize history lists
 * Created: 2021/04/18, Matt Emmerton
 *================================================*/
static void
init_hist_lists (void)
{
	/* V for visit history, planning to also have a change history */
	int count = getdeoptint("HistorySize", 20);
	if (count<0 || count > 9999)
		count = 20;
	init_hist(&vhist, count);
	init_hist(&chist, count);
}

#if !defined(DEADENDS)
/*==================================================
 * load_hist_lists -- Load previous history from database
 * Created: 2001/12/23, Perry Rapp
 *================================================*/
static void
load_hist_lists (void)
{
	if (getdeoptint("SaveHistory", 0)) {
		load_nkey_list("HISTV", &vhist);
		load_nkey_list("HISTC", &chist);
	}
}

/*==================================================
 * save_hist_lists -- Save history into database
 * Created: 2001/12/23, Perry Rapp
 *================================================*/
static void
save_hist_lists (void)
{
	if (!getdeoptint("SaveHistory", 0)) return;
	save_nkey_list("HISTV", &vhist);
	save_nkey_list("HISTC", &chist);
}
#endif

/*==================================================
 * term_hist_lists -- destroy history lists
 * Created: 2021/04/18, Matt Emmerton
 *=================================================*/
static void
term_hist_lists (void)
{
	term_hist(&vhist);
	term_hist(&chist);
}
/*==================================================
 * init_hist -- create & initialize a history list
 * Created: 2001/12/23, Perry Rapp
 *=================================================*/
static void
init_hist (struct hist * histp, int count)
{
	int size = count * sizeof(histp->list[0]);
	memset(histp, 0, sizeof(*histp));
	histp->size = count;
	histp->list = (String *)stdalloc(size);
	memset(histp->list, 0, size);
	histp->start = -1;
	histp->past_end = 0;
}
/*==================================================
 * term_hist -- destroy a history list
 * Created: 2021/04/18, Matt Emmerton
 *=================================================*/
static void
term_hist (struct hist * histp)
{
	stdfree(histp->list);
	histp->size = 0;
}

#if !defined(DEADENDS)
/* currently we have no way to save the history, so loading saving
   history is not an option...  maybe some day we'll figure out a
   location to save it...  */
/*==================================================
 * load_nkey_list -- Load node list from record into NKEY array
 *  key:   [IN]  key used to store list in database
 *  histp: [IN]  history list to save
 * Fills in ndarray with data from database.
 * Created: 2001/12/23, Perry Rapp
 * TODO: should be moved to gedlib
 *================================================*/
static void
load_nkey_list (CString key, struct hist * histp)
{
	String rawrec;
	int32_t * ptr;
	int32_t count;
	int32_t temp;
	int len, i;

	count = 0;
	if (!(rawrec = retrieve_raw_record(key, &len)))
		return;
	if (len < 8 || (len % 8) != 0)
		return;
	ptr = (int32_t *)rawrec;
	temp = *ptr++;
	if (temp<0 || temp > 9999) {
		/* #records failed sanity check */
		msg_error("%s", _(qSbadhistcnt));
		goto end;
	}
	if (temp != *ptr++) {
		/* 2nd copy of #records failed to match */
		msg_error("%s", _(qSbadhistcnt2));
		goto end;
	}
	if (len != (temp+1)*8) {
		/* length should be 8 bytes per record + 8 byte header */
		msg_error("%s", _(qSbadhistlen));
	}
	count = temp;
	if (count > histp->size) count = histp->size;
	for (i=0,temp=0; i<count; ++i) {
		char key[MAXKEYWIDTH+1];
		char ntype = *ptr++;
		int keynum = *ptr++;
		if (!ntype || !keynum)
			continue;
		if (keynum<1 || keynum>MAXKEYNUMBER)
			continue;
		snprintf(key, sizeof(key), "%c" FMT_INT, ntype, keynum);
		strcpy(histp->list[temp].key, key);
		histp->list[temp].ntype = ntype;
		histp->list[temp].keynum = keynum;
		++temp;
	}
	count = temp;
	if (count) {
		histp->start = 0;
		histp->past_end = count % histp->size;
	}

end:
	stdfree(rawrec);
}
#endif

/*==================================================
 * get_hist_count -- Calculate current size of history
 * Created: 2001/12/23, Perry Rapp
 *================================================*/
static int
get_hist_count (struct hist * histp)
{
	if (!histp->size || histp->start==-1)
		return 0;
	else if (histp->past_end > histp->start)
		return histp->past_end - histp->start;
	else
		return histp->size - histp->start + histp->past_end;
}

#if !defined(DEADENDS)
/* currently we do not have a location to save history... */

/*==================================================
 * save_nkey_list -- Save nkey list from circular array
 *  key:     [IN]  key used to store list in database
 *  hist:    [IN]  history list to save
 * Created: 2001/12/23, Perry Rapp
 * TODO: should be moved to gedlib
 *================================================*/
static void
save_nkey_list (CString key, struct hist * histp)
{
	FILE * fp=0;
	int i, next;
	int32_t count;	// write buffer for histp->count value
	int32_t temp;	// write buffer for histp->list[] values
	size_t rtn;

	count = get_hist_count(histp);

	unlink(editfile);

	fp = fopen(editfile, DEWRITETEXT);
	if (!fp) return;

	/* write count first -- twice just to take up 8 bytes same as records */
	rtn = fwrite(&count, 4, 1, fp); ASSERT(rtn==1);
	rtn = fwrite(&count, 4, 1, fp); ASSERT(rtn==1);

	/* write entries */
	next = histp->start;
	for (i=0; i<count; ++i)
	{
		/*
		 * Note that while keynum is an INT, the maximum (integer) key
		 * length allowed in LifeLines is 7 bytes - 9,999,999 - which
		 * is well below the max signed 32-bit integer value, so there
		 * will be no actual data truncation here.
		 */

		/* write type & number, 4 bytes each */
		/* type = char, keynum = INT (truncated!) */
		temp = histp->list[next].ntype;
		rtn = fwrite(&temp, 4, 1, fp); ASSERT(rtn==1);
		temp = histp->list[next].keynum;
		rtn = fwrite(&temp, 4, 1, fp); ASSERT(rtn==1);
		next = (next+1) % histp->size;
		if (next == histp->past_end)
			break; /* finished them all */
	}
	fclose(fp);

	store_text_file_to_db(key, editfile, 0);
}
#endif

/*==================================================
 * history_record_change -- add node to change history
 *  (if different from top of history)
 * Created: 2002/06/23, Perry Rapp
 *================================================*/
void
history_record_change (GNode *rec)
{
	history_record(rec, &chist);
}
/*==================================================
 * history_record -- add node to history if different from top of history
 * Created: 2001/03?, Perry Rapp
 *================================================*/
static void
history_record (GNode *rec, struct hist * histp)
{
	String nkey = 0;
	int next, i;
	int count = get_hist_count(histp);
	int protect = getdeoptint("HistoryBounceSuppress", 0);
	if (!histp->size) return;
	if (histp->start==-1) {
		histp->start = histp->past_end;
		histp->list[histp->start] = nzkey(rec);
		histp->past_end = (histp->start+1) % histp->size;
		return;
	}
	/*
	copy new node into nkey variable so we can check
	if this is the same as our most recent (histp->list[last])
	*/
	nkey = nzkey(rec);
	if (protect<1 || protect>99)
		protect=1;
	if (protect>count)
		protect=count;
	/* traverse from most recent back (bounce suppression) */
	next = (histp->past_end-1);
	if (next < 0) next += histp->size;
	for (i=0; i<protect; ++i) {
		if (strcmp(nkey, histp->list[next]))
			return;
		if (--next < 0) next += histp->size;
	}
	/* it is a new one so add it to circular list */
	histp->list[histp->past_end] = nkey;
	if (histp->start == histp->past_end) {
		/* full buffer & we just overwrote the oldest */
		histp->start = (histp->start+1) % histp->size;
	}
	/* advance pointer to account for what we just added */
	histp->past_end = (histp->past_end+1) % histp->size;
}
/*==================================================
 * history_back -- return prev RECORD in history, if exists
 * Created: 2001/03?, Perry Rapp
 *================================================*/
static GNode *
history_back (struct hist * histp)
{
	int last=0;
	GNode *rec=0;
	if (!histp->size || histp->start==-1)
		return NULL;
	/* back up from histp->past_end to current item */
	last = histp->past_end-1;
	if (last < 0) last += histp->size;
	while (last != histp->start) {
		/* loop is to keep going over deleted ones */
		/* now back up before current item */
		if (--last < 0) last += histp->size;
		rec = getRecord (histp->list[last], currentDatabase->recordIndex);
		if (rec) {
			histp->past_end = (last+1) % histp->size;
			return rec;
		}
	}
	return NULL;
}
/*==================================================
 * history_fwd -- return later NODE in history, if exists
 * Created: 2001/03?, Perry Rapp
 *================================================*/
static GNode *
history_fwd (struct hist * histp)
{
	int next;
	GNode *rec;
	if (!histp->size || histp->past_end == histp->start)
		return NULL; /* at end of full history */
	next = histp->past_end;
	rec = getRecord (histp->list[next], currentDatabase->recordIndex);
	return rec;
}
/*==================================================
 * disp_vhistory_list -- show user the visited history list
 *  returns NULL if no history or if user cancels
 * Created: 2002/06/23, Perry Rapp
 *================================================*/
static GNode *
disp_vhistory_list (void)
{
	return do_disp_history_list(&vhist);
}
/*==================================================
 * disp_chistory_list -- show user the changed history list
 *  returns NULL if no history or if user cancels
 * Created: 2002/06/23, Perry Rapp
 *================================================*/
static GNode *
disp_chistory_list (void)
{
	return do_disp_history_list(&chist);
}
/*==================================================
 * get_chistory_list -- return indiseq of change history
 *  returns NULL if no history
 *================================================*/
Sequence *
get_chistory_list (void)
{
	return get_history_list(&chist);
}
/*==================================================
 * get_vhistory_list -- Return indiseq of visit history
 *  returns NULL if no history
 *================================================*/
Sequence *
get_vhistory_list (void)
{
	return get_history_list(&vhist);
}
/*==================================================
 * do_disp_history_list -- let user choose from history list
 *  calls message(nohist) if none found
 *  returns NULL if no history or if user cancels
 * Created: 2001/04/12, Perry Rapp
 *================================================*/
static GNode *
do_disp_history_list (struct hist * histp)
{
	Sequence *seq = get_history_list(histp);
	GNode *rec=0;

	if (!seq) {
		msg_error("%s", _(qSnohist));
		return NULL;
	}
	rec = chooseFromSequence(seq, DOASK1, _(qSidhist), _(qSidhist), chooseTypeDefault);
	deleteSequence(seq);
	return rec;
}
/*==================================================
 * get_history_list -- return specified history list as indiseq
 *================================================*/
static Sequence *
get_history_list (struct hist * histp)
{
	Sequence *seq=0;
	int next;
	if (!histp->size || histp->start==-1) {
		return NULL;
	}
	/* add all items of history to seq */
	seq = createSequence(currentDatabase->recordIndex);
	next = histp->start;
	while (1) {
		GNode *node=0;
		node = getRecord (histp->list[next], currentDatabase->recordIndex);
		if (node) {
			String key = nxref(node);
			key = strsave(key);
			appendToSequence(seq, key, NULL);
		}
		next = (next+1) % histp->size;
		if (next == histp->past_end)
			break; /* finished them all */
	}
	return seq;
}
/*==================================================
 * ask_clear_history -- delete vist history
 *  (first verify via y/n prompt)
 * Created: 2001/12/23, Perry Rapp
 *================================================*/
static void
ask_clear_history (struct hist * histp)
{
	char buffer[120];
	int count;

	if (!histp->size || histp->start==-1) {
		msg_error("%s", _(qSnohist));
		return;
	}
	count = get_hist_count(histp);
	snprintf(buffer, sizeof(buffer), _(qShistclr), count);
	if (ask_yes_or_no(buffer))
		histp->start = -1;
}
/*==================================================
 * handle_history_cmds -- handle the history commands
 *  returns 0 for not a history command
 *  returns 1 if handled but continue on same page
 *  returns -1 if handled & set *pindi1 for switching pages
 * Created: 2001/04/12, Perry Rapp
 *================================================*/
static int
handle_history_cmds (int c, GNode ** prec1)
{
	GNode *rec=0;
	if (c == CMD_VHISTORY_BACK) {
		rec = history_back(&vhist);
		if (rec) {
			*prec1 = rec;
			return -1; /* handled, change pages */
		}
		msg_error("%s", _(qSnohist));
		return 1; /* handled, stay here */
	}
	if (c == CMD_CHISTORY_BACK) {
		rec = history_back(&chist);
		if (rec) {
			*prec1 = rec;
			return -1; /* handled, change pages */
		}
		msg_error("%s", _(qSnohist));
		return 1; /* handled, stay here */
	}
	if (c == CMD_VHISTORY_FWD) {
		rec = history_fwd(&vhist);
		if (rec) {
			*prec1 = rec;
			return -1; /* handled, change pages */
		}
		msg_error("%s", _(qSnohist));
		return 1; /* handled, stay here */
	}
	if (c == CMD_CHISTORY_FWD) {
		rec = history_fwd(&chist);
		if (rec) {
			*prec1 = rec;
			return -1; /* handled, change pages */
		}
		msg_error("%s", _(qSnohist));
		return 1; /* handled, stay here */
	}
	if (c == CMD_VHISTORY_LIST) {
		rec = disp_vhistory_list();
		if (rec) {
			*prec1 = rec;
			return -1; /* handled, change pages */
		}
		return 1;
	}
	if (c == CMD_CHISTORY_LIST) {
		rec = disp_chistory_list();
		if (rec) {
			*prec1 = rec;
			return -1; /* handled, change pages */
		}
		return 1;
	}
	if (c == CMD_VHISTORY_CLEAR) {
		ask_clear_history(&vhist);
		return 1;
	}
	if (c == CMD_CHISTORY_CLEAR) {
		ask_clear_history(&chist);
		return 1;
	}
	return 0; /* unhandled */
}
/*==================================================
 * add_new_rec_maybe_ref -- add a new record
 *  and optionally create a reference to it from
 *  current record
 * rtns: returns new node if user wants to browse to it
 *       current node to edit it (current node)
 *       NULL to just stay where we are
 * Created: 2001/04/06, Perry Rapp
 *================================================*/
static GNode *
add_new_rec_maybe_ref (GNode *current, char ntype)
{
	GNode *newrec=0;
	GNode *newnode;
	String choices[4];
	char title[60];
	int rtn;

	/* create new node of requested type */
	if (ntype=='E') 
		newrec=edit_add_event();
	else if (ntype=='S')
		newrec=edit_add_source();
	else
		newrec=edit_add_other();
	/* bail if user cancelled creation */
	if (!newrec)
		return NULL;
	newnode = nztop(newrec);
	/* sanity check for long tags in others */
	if (strlen(ntag(newnode))>40) {
		msg_info("%s", _(qStag2lng2cnc));
		return newrec;
	}
	/* now ask the user how to connect the new node */
	snprintf(title, sizeof(title), _(qSnewrecis), nxref(newnode));
	msg_info("%s", title);
	/* keep new node # in status so it will be visible during edit */
	lock_status_msg(true);
	choices[0] = _(qSautoxref);
	choices[1] = _(qSeditcur);
	choices[2] = _(qSgotonew);
	choices[3] = _(qSstaycur);
	rtn = chooseFromArray(NULL, 4, choices);
	lock_status_msg(false);
	switch(rtn) {
	case 0: 
		autoadd_xref(current, newnode);
		return 0;
	case 1:
		return current; /* convention - return current node for edit */
	case 2:
		return newrec;
	default:
		return 0;
	}
}
/*==================================================
 * autoadd_xref -- add trailing xref from existing node
 *  to new node
 * Created: 2001/11/11, Perry Rapp
 *================================================*/
static void
autoadd_xref (GNode *rec, GNode *newnode)
{
	GNode *xref; /* new xref added to end of node */
	GNode *find, *prev; /* used finding last child of node */
	GNode *node = nztop(rec);
	xref = createGNode(NULL, ntag(newnode), nxref(newnode), node);
	if (!(find = nchild(node))) {
		nchild(node) = xref;
	} else {
		/* find last child of node */
		while(find) {
			prev = find;
			find = nsibling(find);
		}
		nsibling(prev) = xref;
	}

	normalizeRecord(nztop(rec));
}

//#if !defined(DEADENDS)
/*==================================================
 * get_vhist_len -- how many records currently in visit history list ?
 * Created: 2002/06/23, Perry Rapp
 *================================================*/
int
get_vhist_len (void)
{
	return get_hist_count(&vhist);
}

/*==================================================
 * get_chist_len -- how many records currently in change history list ?
 * Created: 2002/06/23, Perry Rapp
 *================================================*/
int
get_chist_len (void)
{
	return get_hist_count(&chist);
}
//#endif

/*==================================================
 * init_browse_module -- Do any initialization
 *  This is after database is determined, and before
 *  main_menu begins processing.
 * Created: 2001/12/23, Perry Rapp
 *================================================*/
void
init_browse_module (void)
{
	init_hist_lists();
#if !defined(DEADENDS)
	load_hist_lists();
#endif
}
/*==================================================
 * term_browse_module -- Cleanup for browse module
 *  Primarily to persist history
 * Created: 2001/12/23, Perry Rapp
 *================================================*/
void
term_browse_module (void)
{
#if !defined(DEADENDS)
	save_hist_lists();
#endif
	term_hist_lists();
}

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
 * show.c -- Curses version of display functions
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 24 Aug 93
 *   3.0.0 - 14 Sep 94    3.0.2 - 24 Dec 94
 *   3.0.3 - 03 May 95
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
#include "zstr.h"
#include "translat.h"
#include "rfmt.h"
#include "refnindex.h"
#include "gnode.h"
#include "readwrite.h"
#include "sequence.h"
#include "ask.h"
#include "llinesi.h"
#include "errors.h"
#include "liflines.h"
#include "messages.h"
#include "screen.h"
#include "cscurses.h"
#include "lineage.h"
#include "codesets.h"
#include "stringtable.h"
#include "options.h"
#include "xreffile.h"
#include "de-strings.h"
#include "ll-node.h"
#include "locales.h"
#include "lloptions.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase

/*********************************************
 * global/exported variables
 *********************************************/

int Scroll1=0;

/*********************************************
 * external/imported variables
 *********************************************/

extern bool opt_nocb;	/* TRUE to suppress display of cb. data */

/*********************************************
 * local types
 *********************************************/

typedef char *LINESTRING;
struct tag_prefix {
	String tag;
	String prefix;
};

/*********************************************
 * local enums & defines
 *********************************************/

/* to handle large families, this needs to be made a regular
variable, and checked & resized at init_display_indi time */
#define MAXOTHERS 30

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void add_child_line(int, GNode *node, int width);
static void add_spouse_line(int, GNode *, GNode *, int width);
static bool append_event(String * pstr, String evt, int * plen, int minlen);
static void disp_person_birthdeath(ZSTR zstr, RecordIndexEl *irec, struct tag_prefix * tags, RFMT rfmt);
static void disp_person_name(ZSTR zstr, String prefix, RecordIndexEl *irec, int width);
static void indi_events(String outstr, GNode *indi, int len);
static void init_display_indi(RecordIndexEl *irec, int width);
static void init_display_fam(RecordIndexEl *frec, int width);
static void pedigree_line(CANVASDATA canvas, int x, int y, String string, int overflow);
static String person_display(GNode *, GNode *, int);
static void put_out_line(UIWINDOW uiwin, int x, int y, String string, int maxcol, int flag);
static String sh_fam_to_event_shrt(GNode *node, String tag, String head
	, int len);
static String sh_indi_to_event_long(GNode *node, String tag
	, String head, int len);
static String sh_indi_to_event_shrt(GNode *node, String tag
	, String head, int len);

/*********************************************
 * local variables
 *********************************************/

static LINESTRING Sfath, Smoth, Smarr;
static ZSTR Spers=0, Sbirt=0, Sdeat=0;
static ZSTR Shusb=0, Swife=0;
static ZSTR Shbirt=0, Shdeat=0, Swbirt=0, Swdeat=0;
static LINESTRING Sothers[MAXOTHERS];
static int liwidth;
static int Solen = 0;
static int Scroll2 = 0;
static int number_child_enable = 0;
static struct tag_prefix f_birth_tags[] = {
	{ "BIRT", N_("born") } /* GEDCOM BIRT tag, label to precede date on display */
	,{ "CHR", N_("bapt") } /* GEDCOM CHR tag, label to precede date on display */
	,{ "BAPM", N_("bapt") } /* GEDCOM BAPM tag, label to precede date on display */
	,{ "BARM", N_("barm") } /* GEDCOM BARM tag, label to precede date on display */
	,{ "BASM", N_("basm") } /* GEDCOM BASM tag, label to precede date on display */
	,{ "BLES", N_("bles") }
	,{ "ADOP", N_("adop") }
	,{ "RESI", N_("resi") }
	,{ NULL, NULL }
};
static struct tag_prefix f_death_tags[] = {
	{ "DEAT", N_("died") }
	,{ "BURI", N_("buri") }
	,{ "CREM", N_("crem") }
	,{ NULL, NULL }
};

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*===============================================
 * init_show_module -- Initialize display variables
 *=============================================*/
void
init_show_module (void)
{
	int i;
	liwidth = ll_cols+1;

	Spers = zs_new();
	Sbirt = zs_new();
	Sdeat = zs_new();
	Sfath = (LINESTRING)stdalloc(liwidth);
	Smoth = (LINESTRING)stdalloc(liwidth);
	Smarr = (LINESTRING)stdalloc(liwidth);
	Shusb = zs_new();
	Shbirt = zs_new();
	Shdeat = zs_new();
	Swife = zs_new();
	Swbirt = zs_new();
	Swdeat = zs_new();
	for (i=0; i<MAXOTHERS; i++)
		Sothers[i] = (LINESTRING)stdalloc(liwidth);
	init_disp_reformat();
}
/*===============================================
 * term_show_module -- Free memory used by show module
 *=============================================*/
void
term_show_module (void)
{
	int i;

	zs_free(&Spers);
	zs_free(&Sbirt);
	zs_free(&Sdeat);
	stdfree(Sfath);
	stdfree(Smoth);
	stdfree(Smarr);
	zs_free(&Shusb);
	zs_free(&Shbirt);
	zs_free(&Shdeat);
	zs_free(&Swife);
	zs_free(&Swbirt);
	zs_free(&Swdeat);
	for (i=0; i<MAXOTHERS; i++)
		stdfree(Sothers[i]);
}
/*===============================================
 * disp_person_name -- Display person's name
 *  append REFN or key, & include title if room
 * Created: 2003-01-11 (Perry Rapp)
 *=============================================*/
static void
disp_person_name (ZSTR zstr, String prefix, RecordIndexEl *irec, int width)
{
/* TODO: width handling is wrong, it should not be byte based */
	ZSTR zkey = zs_news(nxref(nztop(irec)));
	/* ": " between prefix and name, and " ()" for key */
	int avail = width - strlen(prefix)-zs_len(zkey)-5;
	String name = personToName(nztop(irec), avail);
	zs_clear(zstr);
	zs_setf(zstr, "%s: %s ", prefix, name);
	avail = width - zs_len(zstr)-zs_len(zkey)-2;
	if (avail > 10) {
		String t = indi_to_title(nztop(irec), avail-3);
		if (t) zs_appf(zstr, "[%s] ", t);
	}
/* TODO: add more names if room */
/* TODO: first implement new function namelist to get us all names */
	if(getdeoptint("DisplayKeyTags", 0) > 0) {
		zs_appf(zstr, "(i%s)", zs_str(zkey));
	} else {
		zs_appf(zstr, "(%s)", zs_str(zkey));
	}
	zs_free(&zkey);
}
/*===============================================
 * disp_person_birthdeath -- Print birth string
 *  Try to find date & place info for birth (or approx)
 * Created: 2003-01-12 (Perry Rapp)
 *=============================================*/
static void
disp_person_birthdeath (ZSTR zstr, RecordIndexEl *irec, struct tag_prefix * tags, RFMT rfmt)
{
	struct tag_prefix *tg;
	int ct=0;
	ZSTR ztemp=zs_new();
	zs_clear(zstr);
	for (tg = tags; tg->tag; ++tg) {
		String date=NULL, place=NULL;
		zs_clear(ztemp);
		ct = 0;
		record_to_date_place(irec, tg->tag, &date, &place, &ct);
		if (!ct) continue;
		if (rfmt && rfmt->rfmt_date) /* format date */
			date = (*rfmt->rfmt_date)(date);
		zs_appf(ztemp, "%s: ", _(tg->prefix));

		if (date) {
			zs_apps(ztemp, date);
		}
		if (place) {
			if (date)
				zs_appf(ztemp, ", ");
			if (rfmt && rfmt->rfmt_plac) /* format place */
				place = (*rfmt->rfmt_plac)(place);
			zs_apps(ztemp, place);
		}
		if (!date && !place) {
			/*
                         * Git #308: INDI with BIRT/DEAT without DATE/PLAC displays "Y"
                         * The 3.0.62 behaviour was to display nothing.
                         * This sounds more appropriate so reverting to that behaviour.
			 */
			/* zs_apps(ztemp, "Y"); */
		}
		if (ct>1) {
			zs_appf(ztemp, " (" FMT_INT " alt)", ct-1);
		}
		/* append current info to accumulated info */
		if (zs_len(zstr)>0) {
			zs_apps(zstr, ", ");
		}
		zs_appz(zstr, ztemp);
	}
	zs_free(&ztemp);
}
/*===============================================
 * init_display_indi -- Initialize display person
 *  Fill in all the local buffers for normal person
 *  display mode (Spers, Sbirt, etc)
 *=============================================*/
static void
init_display_indi (RecordIndexEl *irec, int width)
{
	GNode *pers=nztop(irec);
	GNode *this_fam = 0;
	int nsp, nch, num, nm;
	String s;
	GNode *fth;
	GNode *mth;

	ASSERT(width < ll_cols+1); /* size of Spers etc */


	ASSERT(pers);

	disp_person_name(Spers, _(qSdspl_indi), irec, width);

	disp_person_birthdeath(Sbirt, irec, f_birth_tags, false);

	disp_person_birthdeath(Sdeat, irec, f_death_tags, false);

	fth = personToFather(pers, currentDatabase->recordIndex);
	s = person_display(fth, NULL, width-13);
	if (s) destrncpyf(Sfath, liwidth, uu8, "  %s: %s", _(qSdspl_fath), s);
	else destrncpyf(Sfath, liwidth, uu8, "  %s:", _(qSdspl_fath));

	mth = personToMother(pers, currentDatabase->recordIndex);
	s = person_display(mth, NULL, width-13);
	if (s) destrncpyf(Smoth, liwidth, uu8, "  %s: %s", _(qSdspl_moth), s);
	else destrncpyf(Smoth, liwidth, uu8, "  %s:", _(qSdspl_moth));

	Solen = 0;
	nsp = nch = 0;
	FORSPOUSES(pers, sp, fam, num, database->recordIndex)
		if (sp) add_spouse_line(++nsp, sp, fam, width);
	        if (this_fam != fam) {
		        this_fam = fam; /* only do each family once */
			FORCHILDREN(fam, chld, key, nm, database->recordIndex)
				if(chld) add_child_line(++nch, chld, width);
			ENDCHILDREN
		}
	ENDSPOUSES
}
/*==============================
 * show_indi_vitals -- Display person using
 * the traditional LifeLines vitals format.
 *  uiwin:  [IN] which curses window (usually MAIN_WIN)
 *  pers:   [IN] whom to display
 *  row:    [IN] starting row to draw upon
 *  hgt:    [IN] how many rows to use
 *  width:  [IN] how many columns to use
 *  scroll: [IN] how many rows to skip over at top
 *  reuse:  [IN] flag to avoid recomputing display strings
 * Caller sets reuse flag if it knows that this is the same
 * person displayed last.
 *============================*/
void
show_indi_vitals (UIWINDOW uiwin, RecordIndexEl *irec, LLRECT rect
	, int *scroll, bool reuse)
{
	int i;
	int localrow;
	int overflow;
	WINDOW * win = uiw_win(uiwin);
	int row = rect->top;
	int width = rect->right - rect->left + 1;
	int hgt = rect->bottom - rect->top + 1;

	if (hgt<=0) return;
	if (!reuse)
		init_display_indi(irec, width);
	wipe_window_rect(uiwin, rect);
	if (*scroll) {
		if (*scroll > Solen + 5 - hgt)
			*scroll = Solen + 5 - hgt;
		if (*scroll < 0)
			*scroll = 0;
	}
	/* we keep putting lines out til we run out or exhaust our alloted
	height */
	localrow = row - *scroll;

	mvccwaddnstr(win, row+0, 1, zs_str(Spers), width-1);
	if (hgt==1) return;
	mvccwaddnstr(win, row+1, 1, zs_str(Sbirt), width-1);
	if (hgt==2) return;
	mvccwaddnstr(win, row+2, 1, zs_str(Sdeat), width-1);
	if (hgt==3) return;
	mvccwaddnstr(win, row+3, 1, Sfath, width-1);
	if (hgt==4) return;
	mvccwaddnstr(win, row+4, 1, Smoth, width-1);
	if (hgt==5) return;
	for (i = *scroll; i < Solen && i < hgt-5+ *scroll; i++)
	{
		/* the other lines scroll internally, and we
		mark the top one displayed if not the actual top one, and the
		bottom one displayed if not the actual bottom */
		overflow = ((i+1 == hgt-5+ *scroll)&&(i+1 != Solen));
		if (*scroll && (i == *scroll))
			overflow = 1;
		put_out_line(uiwin, localrow+5+i, rect->left, Sothers[i], rect->right, overflow);
	}
}
/*=============================================
 * add_spouse_line -- Add spouse line to others
 *===========================================*/
static void
add_spouse_line (ATTRIBUTE_UNUSED int num, GNode *indi, GNode *fam, int width)
{
	String line, ptr=Sothers[Solen];
	int mylen=liwidth;
	if (Solen >= MAXOTHERS) return;
	if (mylen>width) mylen=width;
	destrcatn(&ptr, " ", &mylen);
	destrcatn(&ptr, _(qSdspl_spouse), &mylen);
	destrcatn(&ptr, ": ", &mylen);
	line = person_display(indi, fam, mylen-1);
	destrcatn(&ptr, line, &mylen);
	++Solen;
}
/*===========================================
 * add_child_line -- Add child line to others
 *=========================================*/
static void
add_child_line (int num, GNode *node, int width)
{
	String line;
	String child = _(qSdspl_child);
	if (Solen >= MAXOTHERS) return;
	line = person_display(node, NULL, width-15);
	if (number_child_enable)
		destrncpyf(Sothers[Solen], liwidth, uu8, "  " FMT_INT_2 "%s: %s", num, child, line);
	else
		destrncpyf(Sothers[Solen], liwidth, uu8, "  "           "%s: %s",      child, line);
	Sothers[Solen++][width-2] = 0;
}
/*==============================================
 * init_display_fam -- Initialize display family
 *============================================*/
static void
init_display_fam (RecordIndexEl *frec, int width)
{
	GNode *fam=nztop(frec);
	GNode *husb=0, *wife=0;
	String s=0;
	ZSTR famkey = zs_news(nxref(fam));
	int nch, nm, wtemp;
	String father = _(qSdspl_fath);
	String mother = _(qSdspl_moth);
	RecordIndexEl *ihusb=0, *iwife=0;
	int husbstatus = 0;
	int wifestatus = 0;
	GNode *fnode;

	/* Get the first two spouses in the family and use them rather than
	 * displaying first husband and first mother
	 * This causes a more reasonable presentation of non-traditional
	 * familes.  Also it will display first hustband and first wife
	 * for traditional families (as there's only one) and the db routines
	 * insert HUSB records before WIFE records.
	 */
	if (fam) {
		fnode = nchild(fam);
		husbstatus = next_spouse(&fnode,&ihusb, currentDatabase);
		husb = nztop(ihusb);
		if (fnode) {
			fnode = nsibling(fnode);
			wifestatus = next_spouse(&fnode,&iwife, currentDatabase);
			wife = nztop(iwife);
		}
	}
	/* if the only spouse is female, list in second slot
	 * hiding the non-traditional behavior
	 */
	if (!wife && husb && SEXV(husb) == sexFemale)
	{
		wife = husb;
		husb = 0;
		iwife = ihusb;
		ihusb = 0;
		wifestatus = husbstatus;
		husbstatus  = 0;
	}

	if (husbstatus == 1) {
		int avail = width - zs_len(famkey) - 3;
		disp_person_name(Shusb, SEXV(husb)==sexMale?father:mother, ihusb, avail);
	} else {
		zs_setf(Shusb, "%s:", father);
		if (husbstatus == -1)
			zs_apps(Shusb, "??");
	}
	if(getdeoptint("DisplayKeyTags", 0) > 0) {
		zs_appf(Shusb, " (f%s)", zs_str(famkey));
	} else {
		zs_appf(Shusb, " (%s)", zs_str(famkey));
	}
	zs_free(&famkey);

	disp_person_birthdeath(Shbirt, ihusb, f_birth_tags, false);
	disp_person_birthdeath(Shdeat, ihusb, f_death_tags, false);

	if (wifestatus == 1) {
		int avail = width;
		disp_person_name(Swife, SEXV(wife)==sexMale?father:mother, iwife, avail);
	} else {
		zs_setf(Swife, "%s:", mother);
		if (wifestatus == -1)
			zs_apps(Swife, "??");
	}

	disp_person_birthdeath(Swbirt, iwife, f_birth_tags, false);
	disp_person_birthdeath(Swdeat, iwife, f_death_tags, false);

	/* Find marriage (or marital contract, or engagement) */
	s = sh_indi_to_event_long(fam, "MARR", _(qSdspl_mar), width-3);
	if (!s) s = sh_indi_to_event_long(fam, "MARC", _(qSdspl_marc), width-3);
	if (!s) s = sh_indi_to_event_long(fam, "ENGA", _(qSdspl_eng), width-3);
	if (s) destrncpyf(Smarr, liwidth, uu8, "%s", s);
	else destrncpyf(Smarr, liwidth, uu8, "%s", _(qSdspl_mar));

	/* append divorce to marriage line, if room */
	/* (Might be nicer to make it a separate, following line */
	wtemp = width-5 - strlen(Smarr);
	if (wtemp > 10) {
		s = sh_indi_to_event_long(fam, "DIV", _(qSdspa_div), wtemp);
		if (s)
			destrncpyf(Smarr+strlen(Smarr), liwidth-strlen(Smarr), uu8, ", %s", s);
	}

	Solen = 0;
	nch = 0;
	FORCHILDREN(fam, chld, key, nm, database->recordIndex)
	  add_child_line(++nch, chld, width);
	ENDCHILDREN
	releaseRecord(ihusb);
	releaseRecord(iwife);
}
/*===================================
 * show_fam_vitals -- Display family
 * [in] fam:  whom to display
 * [in] row:   starting row to use
 * [in] hgt:   how many rows allowed
 * [in] width: how many columns allowed
 * [in] reuse: flag to save recalculating display strings
 *=================================*/
void
show_fam_vitals (UIWINDOW uiwin, RecordIndexEl *frec, int row, int hgt
	, int width, int *scroll, bool reuse)
{
	int i;
	int localrow;
	int overflow;
	char buf[132];
	int maxcol = width-1;
	WINDOW * win = uiw_win(uiwin);
	struct tag_llrect rect;

	rect.bottom = row+hgt-1;
	rect.top = row;
	rect.left = 1;
	rect.right = width-2;

	if (!reuse)
		init_display_fam(frec, width);
	wipe_window_rect(uiwin, &rect);
	if (*scroll) {
		if (*scroll > Solen + 7 - hgt)
			*scroll = Solen + 7 - hgt;
		if (*scroll < 0)
			*scroll = 0;
	}
	localrow = row - *scroll;
	mvccwaddnstr(win, row+0, 1, zs_str(Shusb), width-2);
	if (hgt==1) return;
	mvccwaddnstr(win, row+1, 1, zs_str(Shbirt), width-2);
	if (hgt==2) return;
	mvccwaddnstr(win, row+2, 1, zs_str(Shdeat), width-2);
	if (hgt==3) return;
	mvccwaddnstr(win, row+3, 1, zs_str(Swife), width-2);
	if (hgt==4) return;
	mvccwaddnstr(win, row+4, 1, zs_str(Swbirt), width-2);
	if (hgt==5) return;
	mvccwaddnstr(win, row+5, 1, zs_str(Swdeat), width-2);
	if (hgt==6) return;
	mvccwaddnstr(win, row+6, 1, Smarr, width-2);
	for (i = *scroll; i < Solen && i < hgt-7+*scroll; i++)
	{
		overflow = ((i+1 == hgt-7+*scroll)&&(i+1 != Solen));
		if (*scroll && (i == *scroll))
			overflow = 1;
		put_out_line(uiwin, localrow+7+i, 1, Sothers[i]+1, maxcol, overflow);
	}
}
/*================================================
 * show_ancestors -- Show pedigree/ancestors
 * Created: 2001/02/04, Perry Rapp
 *==============================================*/
void
show_ancestors (UIWINDOW uiwin, RecordIndexEl *irec, LLRECT rect
	, int * scroll, bool reuse)
{
	struct tag_canvasdata canvas;
		/* parameters for drawing tree */
	canvas.rect = rect;
	canvas.scroll = *scroll;
	canvas.param = (void *)uiwin;
	canvas.line = pedigree_line;
		/* clear & draw pedigree */
	wipe_window_rect(main_win, rect);
	pedigree_draw_ancestors(irec, &canvas, reuse);
	*scroll = canvas.scroll;
}
/*================================================
 * show_descendants -- Show pedigree/descendants
 * Created: 2001/02/04, Perry Rapp
 *==============================================*/
void
show_descendants (UIWINDOW uiwin, RecordIndexEl *rec, LLRECT rect
	, int * scroll, bool reuse)
{
	struct tag_canvasdata canvas;
		/* parameters for drawing tree */
	canvas.rect = rect;
	canvas.scroll = *scroll;
	canvas.param = (void *)uiwin;
	canvas.line = pedigree_line;
		/* clear & draw pedigree */
	wipe_window_rect(main_win, rect);
	pedigree_draw_descendants(rec, &canvas, reuse);
	*scroll = canvas.scroll;
}
/*================================================
 * show_gedcom -- Display record in raw gedcom format
 * Created: 2001/01/27, Perry Rapp
 *==============================================*/
void
show_gedcom (UIWINDOW uiwin, RecordIndexEl *rec, int gdvw, LLRECT rect
	, int * scroll, bool reuse)
{
	struct tag_canvasdata canvas;
		/* parameters for drawing */
	canvas.rect = rect;
	canvas.scroll = *scroll;
	canvas.param = (void *)uiwin;
	canvas.line = pedigree_line;
		/* clear & draw pedigree */
	wipe_window_rect(uiwin, rect);
	pedigree_draw_gedcom(rec, gdvw, &canvas, reuse);
	*scroll = canvas.scroll;
}
/*================================================
 * switch_scrolls -- Interchange scroll1 & scroll2
 * This is how the tandem modes do their drawing of
 * the lower part -- they swap in the second set of
 * scroll briefly for displaying the lower part, then
 * swap back to normal as soon as finishing lower part.
 * Created: 2001/02/04, Perry Rapp
 *==============================================*/
void
switch_scrolls (void)
{
	int save = Scroll1;
	Scroll1 = Scroll2;
	Scroll2 = save;
}
/*===============================================================
 * indi_to_ped_fix -- Construct person String for pedigree screen
 * returns static buffer
 *  indi: [in] person to display
 *  len:  [in] max width
 * Does internal-to-display translation
 *=============================================================*/
String
indi_to_ped_fix (GNode *indi, int len)
{
	String bevt, devt, name, key;
	int tmp1_length, name_length;
	static char scratch[200];
	char tmp1[200]; /* holds birth, death, key string */

	if (!indi) return (String) "------------";
	bevt = eventToDate(BIRT(indi), true);
	if (!bevt) bevt = eventToDate(BAPT(indi), true);
	if (!bevt) bevt = (String) "";
	devt = eventToDate(DEAT(indi), true);
	if (!devt) devt = eventToDate(BURI(indi), true);
	if (!devt) devt = (String) "";
	if (keyflag) {
		key = nxref(indi);
		if(getdeoptint("DisplayKeyTags", 0) > 0) {
			snprintf(tmp1, sizeof(tmp1), " [%s-%s] (i%s)", bevt, devt, key);
		} else {
			snprintf(tmp1, sizeof(tmp1), " [%s-%s] (%s)", bevt, devt, key);
		}
	}
	else
	{
		snprintf(tmp1, sizeof(tmp1), " (%s-%s)", bevt, devt);
	}
	tmp1[ARRAYSIZE(tmp1) - 1] = 0;
	
	/* a long name may need to be truncated to fit on the screen */
	len = min(len, ((int)ARRAYSIZE(scratch) - 1));
	tmp1_length = (int)strlen(tmp1);
	name_length = len - tmp1_length - 1;
	name_length = max(0, name_length);
	name = personToName(indi, name_length);
	ASSERT(name_length + tmp1_length < (int)ARRAYSIZE(scratch));
	strcpy(scratch, name);
	strcat(scratch, tmp1);
	return scratch;
}
/*=============================================
 * append_event -- Add an event if present to output string
 *  pstr:   [I/O] end of printed event string 
 *  evt:    [IN]  event string (must be valid)
 *  plen:   [I/O] max length of output
 *  minlen: [IN]  threshold for caller to stop
 * If event found, this prints it, advances *pstr, and reduces *plen
 * returns FALSE if (*plen)<minlen after advancing
 *  (signal to caller to stop)
 * Created: 2001/07/04 (Perry Rapp)
 *===========================================*/
static bool
append_event (String * pstr, String evt, int * plen, int minlen)
{
	destrcatn(pstr, ", ", plen);
	destrcatn(pstr, evt, plen);
	return *plen >= minlen;
}
/*=============================================
 * family_events -- Print string of events
 *  outstr: [I/O] printed event string 
 *  indi:   [IN]  whom to display
 *  fam:    [IN]  family record (used when displaying spouses)
 *  len:    [IN]  max length of output
 * If none are found, this will write a 0 to first char of outstr
 * If anything written, starts with ", "
 * Created: 2001/07/04 (Perry Rapp)
 *===========================================*/
static void
family_events (String outstr, GNode *indi, GNode *fam, int len)
{
	String evt = NULL;
	String p = outstr;
	int mylen = len;
	p[0] = 0;

	/* find marriage (or marital contract, or engagement) */
	if (!evt) {
		evt = sh_fam_to_event_shrt(fam, "MARR", _(qSdspa_mar), mylen);
		if (evt && !append_event(&p, evt, &mylen, 10))
			return;
	}
	if (!evt) {
		evt = sh_fam_to_event_shrt(fam, "MARC", _(qSdspa_marc), mylen);
		if (evt && !append_event(&p, evt, &mylen, 10))
			return;
	}
	if (!evt) {
		evt = sh_fam_to_event_shrt(fam, "ENGA", _(qSdspa_eng), mylen);
		if (evt && !append_event(&p, evt, &mylen, 10))
			return;
	}

/*
	mylen is up-to-date how many chars left we have
	(we keep passing mylen-2 because events are prefixed with ", ")
	if we ever have too few left (<10), append_event will return FALSE
*/
	if (!opt_nocb) {
		GNode *chld;
		/* Look for birth or christening of first child */
		if (chld = familyToFirstChild(fam, currentDatabase->recordIndex)) {
			evt = sh_indi_to_event_shrt(chld, "BIRT", _(qSdspa_chbr), mylen-2);
			if (evt && !append_event(&p, evt, &mylen, 10))
				return;
			if (!evt) {
				evt = sh_indi_to_event_shrt(chld, "CHR", _(qSdspa_chbr), mylen-2);
				if (evt && !append_event(&p, evt, &mylen, 10))
					return;
			}
		}
	}
	evt = sh_indi_to_event_shrt(indi, "BIRT", _(qSdspa_bir), mylen-2);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	evt = sh_indi_to_event_shrt(indi, "CHR", _(qSdspa_chr), mylen-2);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	evt = sh_indi_to_event_shrt(indi, "DEAT", _(qSdspa_dea), mylen-2);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
	evt = sh_indi_to_event_shrt(indi, "BURI", _(qSdspa_bur), mylen-2);
	if (evt && !append_event(&p, evt, &mylen, 10))
		return;
}
/*=============================================
 * indi_events -- Print string of events
 *  outstr: [I/O] printed event string 
 *  indi:   [IN]  whom to display
 *  len:    [IN]  max length of output
 * If none are found, this will write a 0 to first char of outstr
 * If anything written, starts with ", "
 * Created: 2001/07/04 (Perry Rapp)
 *===========================================*/
static void
indi_events (String outstr, GNode *indi, int len)
{
	String evt = NULL;
	int width = (len-2)/2;
	String p = outstr;
	int mylen = len;
	p[0] = 0;

	evt = sh_indi_to_event_shrt(indi, "BIRT", _(qSdspa_bir), width);
	if (!evt)
		evt = sh_indi_to_event_shrt(indi, "CHR", _(qSdspa_chr), width);
	if (evt) {
		destrcatn(&p, ", ", &mylen);
		destrcatn(&p, evt, &mylen);
	}
	if (p == outstr)
		width = len;
	evt = sh_indi_to_event_shrt(indi, "DEAT", _(qSdspa_dea), width);
	if (!evt) evt = sh_indi_to_event_shrt(indi, "BURI", _(qSdspa_bur), width);
	if (evt) {
		destrcatn(&p, ", ", &mylen);
		destrcatn(&p, evt, &mylen);
	}
}
/*==========================================================
 * max_keywidth -- Figure the width of the widest extant key
 *========================================================*/
static int
max_keywidth (void)
{
	int32_t maxkey = xref_max_any();
	if (maxkey>9999) {
		if (maxkey>999999)
			return 7;
		if (maxkey>99999)
			return 6;
		return 5;
	}
	if (maxkey>999)
		return 4;
	if (maxkey>99)
		return 3;
	return 2;
}
/*=============================================
 * person_display -- Create person display line
 *  indi:  [in] whom to display
 *  fam:   [in] family record (used when displaying spouses)
 *  len:   max length of output
 *===========================================*/
static String
person_display (GNode *indi, GNode *fam, int len)
{
	static char scratch1[120];
	static char scratch2[100];
	String p;
	/* parentheses & leading space & possible "i" */
	int keyspace = max_keywidth() + 4; 
	int evlen, namelen, temp;
	/* don't overflow scratch1, into which we catenate name & events */
	if (len > (int)ARRAYSIZE(scratch1)-1)
		len = ARRAYSIZE(scratch1)-1;

	/* keywidth for key, 2 for comma space, and split between name & events */
	evlen = (len-2-keyspace)/2;
	namelen = evlen;

	if (!indi) return NULL;

	/* test to see if name is short */
	p = personToName(indi, 100);
	if ((temp = strlen(p)) < evlen) {
		/* name is short, give extra to events */
		evlen += (namelen - temp);
		namelen -= (namelen - temp);
	}

	if (evlen > (int)ARRAYSIZE(scratch2)-1) /* don't overflow name buffer */
		evlen = ARRAYSIZE(scratch2)-1;
	if (fam) {
		family_events(scratch2, indi, fam, evlen);
	} else {
		indi_events(scratch2, indi, evlen);
	}

	/* give name any unused space events left */
	if ((int)strlen(scratch2)<evlen)
		namelen += evlen-(int)strlen(scratch2);
	p = scratch1;
	strcpy(p, personToName(indi, namelen));
	p += strlen(p);
	if (scratch2[0]) {
		strcpy(p, scratch2);
		p += strlen(p);
	}
	if(getdeoptint("DisplayKeyTags", 0) > 0) {
		snprintf(p, scratch1+len-p, " (i%s)", nxref(indi));

	} else {
		snprintf(p, scratch1+len-p, " (%s)", nxref(indi));
	}
	return scratch1;
}
/*========================================================
 * show_aux -- Show source, event or other record
 *======================================================*/
void
show_aux (UIWINDOW uiwin, RecordIndexEl *rec, int mode, LLRECT rect
	, int * scroll, bool reuse)
{
	if (mode == 'g')
		show_gedcom(uiwin, rec, GDVW_NORMAL, rect, scroll, reuse);
	else if (mode == 't')
		show_gedcom(uiwin, rec, GDVW_TEXT, rect, scroll, reuse);
	else
		show_gedcom(uiwin, rec, GDVW_EXPANDED, rect, scroll, reuse);
}
/*===============================================
 * show_scroll - vertically scroll person display
 *=============================================*/
void
show_scroll (int delta)
{
	Scroll1 += delta;
	if (Scroll1 < 0)
		Scroll1 = 0;
}
/*===================================
 * show_scroll2 - scroll lower window
 *  (in tandem mode)
 *=================================*/
void
show_scroll2 (int delta)
{
	Scroll2 += delta;
	if (Scroll2 < 0)
		Scroll2 = 0;
}
/*=================================
 * show_reset_scroll - clear scroll
 *===============================*/
void
show_reset_scroll (void)
{
	Scroll1 = 0;
	Scroll2 = 0;
}
/*=====================================
 * pedigree_line - callback from pedigree code
 *  to put out each line of pedigree
 *====================================*/
static void
pedigree_line (CANVASDATA canvas, int y, int x, String string, int overflow)
{
	if (!string || !string[0]) return;
	/* vertical clip to rect */
	if (y < canvas->rect->top || y > canvas->rect->bottom) return;
	/* horizontal clip to rect */
	if (x < canvas->rect->left) {
		int delta = canvas->rect->left - x;
		if ((int)strlen(string) <= delta) return;
		string += delta;
		x = canvas->rect->left;
	}
	put_out_line((UIWINDOW)canvas->param, y, x, string, canvas->rect->right, overflow);
}
/*=====================================
 * put_out_line - move string to screen
 * but also append ++ at end if flagged
 * start string at x,y, and do not go beyond maxcol
 * _disp version means string is already in display encoding
 *====================================*/
static void
put_out_line (UIWINDOW uiwin, int y, int x, String string, int maxcol, int flag)
{
	WINDOW * win = uiw_win(uiwin);
	int buflen = (maxcol - x + 1) + 1;
	LINESTRING buffer = (LINESTRING)stdalloc(buflen);

	/* TODO: Should convert to output codeset now, before limiting text */

	/* copy into local buffer (here we enforce maxcol) */
	destrncpy(buffer, string, buflen, uu8);
	if (flag) {
		/* put ++ against right, padding if needed */
		int i = strlen(buffer);
		int pos = maxcol-x-3;
		if (i>pos)
			i = pos;
		for (; i<pos; i++)
			buffer[i] = ' ';
		buffer[i++] = ' ';
		buffer[i++] = '+';
		buffer[i++] = '+';
		buffer[i++] = '\0';
	}
	mvccwaddstr(win, y, x, buffer);
	stdfree(buffer);
}
/*==================================================================
 * show_childnumbers - toggle display of numbers for children
 *================================================================*/
void
show_childnumbers (void)
{
	number_child_enable = !number_child_enable;
}
/*================================================
 * sh_indi_to_event -- Pass-thru to indi_to_event
 *  using long display reformatting
 *==============================================*/
static String
sh_indi_to_event_long (GNode *node, String tag, String head, int len)
{
	return personToEvent(node, tag, head, len, false);
}
/*================================================
 * sh_indi_to_event_shrt -- Pass-thru to indi_to_event, short display
 *  using short display reformatting
 *==============================================*/
static String
sh_indi_to_event_shrt (GNode *node, String tag, String head, int len)
{
	return personToEvent(node, tag, head, len, true);
}
/*==================================================
 * sh_fam_to_event_shrt -- Pass-thru to fam_to_event
 *  using display reformatting
 *================================================*/
static String
sh_fam_to_event_shrt (GNode *node, String tag, String head, int len)
{
	return familyToEvent(node, tag, head, len, true);
}

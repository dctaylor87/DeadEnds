/* 
   Copyright (c) 2006 Perry Rapp

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
 * searchui.c -- menus for full database search
 *  (see scan.c for code that actually scans entire database)
 * Copyright(c) 2006
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
#include "hashtable.h"
#include "refnindex.h"
#include "database.h"
#include "gnode.h"
#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "xlat.h"
#include "readwrite.h"

#include "rfmt.h"
#include "sequence.h"
#include "ask.h"
#include "llinesi.h"
#include "errors.h"
#include "liflines.h"
#include "messages.h"
#include "screen.h"
#include "screeni.h"
#include "cscurses.h"
#include "codesets.h"
#include "de-strings.h"
#include "ui.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static Sequence *invoke_fullscan_menu(void);
static void repaint_fullscan_menu(UIWINDOW uiwin);
static Sequence *invoke_search_source_menu(void);
static void repaint_search_menu(UIWINDOW uiwin);
static void repaint_search_source_menu(UIWINDOW uiwin);

/*********************************************
 * local variables
 *********************************************/

static UIWINDOW search_menu_win=NULL;
static UIWINDOW fullscan_menu_win=NULL;
static UIWINDOW search_source_menu_win=NULL;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==============================
 * invoke_search_menu -- Handle search menu
 * If return set has one element, it has already been confirmed
 *============================*/
Sequence *
invoke_search_menu (void)
{
	UIWINDOW uiwin=0;
	Sequence *seq=0;
	int code=0;
	bool done=false;

	if (!search_menu_win) {
		create_newwin2(&search_menu_win, "search_menu", 7,66);
	}
	/* repaint it every time, as history counts change */
	repaint_search_menu(search_menu_win);
	uiwin = search_menu_win;

	while (!done) {
		activate_uiwin(uiwin);
		place_cursor_popup(uiwin);
		code = interact_choice_string(uiwin, "vcfq");

		switch (code) {
		case 'v':
			seq = get_vhistory_list();
			break;
		case 'c':
			seq = get_chistory_list();
			if (seq)
				done=true;
			break;
		case 'f':
			seq = invoke_fullscan_menu();
			break;
		case 'q': 
			done=true;
			break;
		}
		if (seq)
			done=true;
		deactivate_uiwin_and_touch_all();
	}
	return seq;
}
/*=====================================
 * repaint_search_menu -- Draw menu for main history/scan menu
 *===================================*/
static void
repaint_search_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	int row = 1;
	String title = _(qSmn_sea_ttl);
	int n = 0;
	char buffer[80];
	draw_win_box(win);
	mvccwaddstr(win, row++, 2, title);
	n = get_vhist_len();
	if (n>0) {
		destrncpyf(buffer, sizeof(buffer), uu8
			, _pl("v  Review visit history (" FMT_INT " record)"
			, "v  Review visit history (" FMT_INT " records)"
			, n), n);
	} else {
		destrncpy(buffer, _("(visit history is empty)"), sizeof(buffer), uu8);
	}
	mvccwaddstr(win, row++, 4, buffer);
	n = get_chist_len();
	if (n>0) {
		destrncpyf(buffer, sizeof(buffer), uu8
			, _pl("c  Review change history (" FMT_INT " record)"
			, "c  Review change history (" FMT_INT " records)"
			, n), n);
	} else {
		destrncpy(buffer, _("(change history is empty)")
			, sizeof(buffer), uu8);
	}
	mvccwaddstr(win, row++, 4, buffer);
	mvccwaddstr(win, row++, 4, _("f  Full database scan"));
	mvccwaddstr(win, row++, 4, _(qSmn_ret));
	/* set cursor position */
	uiw_cury(uiwin) = 1;
	uiw_curx(uiwin) = 3+strlen(title);
}
/*==============================
 * invoke_fullscan_menu -- Handle fullscan menu
 * If return set has one element, it has already been confirmed
 *============================*/
static Sequence *
invoke_fullscan_menu (void)
{
	UIWINDOW uiwin=0;
	Sequence *seq=0;
	int code=0;
	bool done=false;

	if (!fullscan_menu_win) {
		create_newwin2(&fullscan_menu_win, "fullscan", 8, 66);
		/* paint it for the first & only time (it's static) */
		repaint_fullscan_menu(fullscan_menu_win);
	}
	uiwin = fullscan_menu_win;

	while (!done) {
		activate_uiwin(uiwin);
		place_cursor_popup(uiwin);
		code = interact_choice_string(uiwin, "fnrsq");

		switch (code) {
		case 'f':
			seq = fullNameScan(_(qSsts_sca_ful), currentDatabase);
			break;
		case 'n':
			seq = nameFragmentScan(_(qSsts_sca_fra), currentDatabase);
			break;
		case 'r':
			seq = refnScan(_(qSsts_sca_ref), currentDatabase);
			break;
		case 's':
			seq = invoke_search_source_menu();
			break;
		case 'q': 
			done=true;
			break;
		}
		if (seq) {
			if (lengthSequence(seq) > 0) {
				done=true;
			} else {
				deleteSequence(seq);
				seq = NULL;
			}
		}
		deactivate_uiwin_and_touch_all();
		if (!done)
			msg_status("%s", _(qSsts_sca_non));
	}
	return seq;
}
/*=====================================
 * repaint_fullscan_menu -- Draw menu choices for main full scan menu
 *===================================*/
static void
repaint_fullscan_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	int row = 1;
	String title = _("What scan type?");
	draw_win_box(win);
	mvccwaddstr(win, row++, 2, title);
	mvccwaddstr(win, row++, 4, _("f  Full name scan"));
	mvccwaddstr(win, row++, 4, _("n  Name fragment (whitespace-delimited) scan"));
	mvccwaddstr(win, row++, 4, _("r  Refn scan"));
	mvccwaddstr(win, row++, 4, _("s  Source scan"));
	mvccwaddstr(win, row++, 4, _(qSmn_ret));
	/* set cursor position */
	uiw_cury(uiwin) = 1;
	uiw_curx(uiwin) = 3+strlen(title);
}
/*==============================
 * invoke_search_source_menu -- Handle fullscan menu
 * If return set has one element, it has already been confirmed
 *============================*/
static Sequence *
invoke_search_source_menu (void)
{
	UIWINDOW uiwin=0;
	Sequence *seq=0;
	int code=0;
	bool done=false;

	if (!search_source_menu_win) {
		create_newwin2(&search_source_menu_win, "search_source", 7 ,66);
		/* paint it for the first & only time (it's static) */
		repaint_search_source_menu(search_source_menu_win);
	}
	uiwin = search_source_menu_win;

	while (!done) {
		activate_uiwin(uiwin);
		place_cursor_popup(uiwin);
		code = interact_choice_string(uiwin, "atq");

		switch (code) {
		case 'a':
			seq = scanSourceByAuthor(_(qSsts_sca_src), currentDatabase);
			break;
		case 't':
			seq = scanSourceByTitle(_(qSsts_sca_src), currentDatabase);
			break;
		case 'q': 
			done=true;
			break;
		}
		if (seq) {
			if (lengthSequence(seq) > 0) {
				done=true;
			} else {
				deleteSequence(seq);
				seq = NULL;
			}
		}

		deactivate_uiwin_and_touch_all();
		if (!done)
			msg_status("%s", _(qSsts_sca_non));
	}
	return seq;
}
/*=====================================
 * repaint_search_source_menu -- Draw menu choices for searching all sources
 *===================================*/
static void
repaint_search_source_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	int row = 1;
	String title = _("Scan on what source field?");
	draw_win_box(win);
	mvccwaddstr(win, row++, 2, title);
	mvccwaddstr(win, row++, 4, _("a  Scan by author"));
	mvccwaddstr(win, row++, 4, _("t  Scan by title"));
	mvccwaddstr(win, row++, 4, _(qSmn_ret));
	/* set cursor position */
	uiw_cury(uiwin) = 1;
	uiw_curx(uiwin) = 3+strlen(title);
}

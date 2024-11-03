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
 * screen.c -- Curses user interface to DeadEnds
 * Copyright(c) 1992-96 by T.T. Wetmore IV; all rights reserved
 *===========================================================*/

#include <ansidecl.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "denls.h"
#include "refnindex.h"
#include "gnode.h"
#include "readwrite.h"
#include "options.h"

#include "rfmt.h"
#include "sequence.h"
#include "hashtable.h"
#include "stringtable.h"
#include "zstr.h"
#include "translat.h"
#include "xlat.h"
#include "ask.h"
#include "choose.h"
#include "llinesi.h"
#include "errors.h"
#include "liflines.h"
#include "menuitem.h"
#include "messages.h"
#include "screen.h"
#include "screeni.h"
#include "curses-ui.h"
#include "cscurses.h"
#include "messages.h"
#include "codesets.h"
#include "listui.h"
#include "list.h"
#include "locales.h"
#include "database.h"
//#include "readindex.h"
#include "interp.h"		/* interpret, programRunning */
#include "charprops.h"
#include "llgettext.h"
#include "llpy-externs.h"
#include "ll-list.h"
#include "locales.h"
#include "de-strings.h"
#include "ui.h"
#include "lloptions.h"
#include "version.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase

#define LINESREQ 24
#define COLSREQ  80
/*
OVERHEAD_MENU:
1 line across top of screen
1 line above menu
1 line below menu
1 line of prompt message
1 line at very bottom of screen
This is how many lines can't be used by person (or whatever)
*/
#define OVERHEAD_MENU 5
int MAINWIN_WIDTH=0;

/* center windows on real physical screen (LINES x COLS) */
#define NEWWIN(r,c)   newwin(r,c,(LINES - (r))/2,(COLS - (c))/2)
#define SUBWIN(w,r,c) subwin(w,r,c,(LINES - (r))/2,(COLS - (c))/2)

/*********************************************
 * global/exported variables
 *********************************************/

int ll_lines = -1; /* update to be number of lines in screen */
int ll_cols = -1;	 /* number of columns in screen used by DeadEnds */
bool stdout_vis = false;
int cur_screen = 0;
UIWINDOW main_win = NULL;
UIWINDOW stdout_win=NULL;
static UIWINDOW debug_win=NULL, debug_box_win=NULL;
static UIWINDOW ask_win=NULL, ask_msg_win=NULL;
static UIWINDOW add_menu_win=NULL, del_menu_win=NULL;
static UIWINDOW utils_menu_win=NULL, tt_menu_win=NULL;
static UIWINDOW extra_menu_win=NULL;

/*********************************************
 * external/imported variables
 *********************************************/

extern int alldone;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void add_shims_info(List *list);
static void add_uiwin(UIWINDOW uiwin);
static void append_to_msg_list(String msg);
//static bool ask_for_filename_impl(String ttl, String path, String prmpt
//	, String buffer, int buflen);
static void begin_action(void);
static void check_menu(DYNMENU dynmenu);
static void check_stdout(void);
static int chooseOrViewArray (CString ttl, int no, String *pstrngs
	, bool selecting, DETAILFNC detfnc, void *param);
static int choose_tt(String prompt);
static void clear_msgs(void);
static void clear_status(void);
static void clearw(void);
static void color_hseg(WINDOW *win, int row, int x1, int x2, char ch);
static void create_boxed_newwin2(UIWINDOW * puiw, CString name, int rows, int cols);
static void create_newwin(UIWINDOW * puiw, CString name, int rows, int cols, int begy, int begx);
static void create_uisubwindow(UIWINDOW * puiw, CString name, UIWINDOW parent, int rows, int cols, int begy, int begx);
static void create_uiwindow_impl(UIWINDOW * puiw, CString name, WINDOW * win, int rows, int cols);
static void create_windows(void);
static void deactivate_uiwin(void);
static void delete_uiwindow_impl(UIWINDOW uiw);
static void delete_uiwindow(UIWINDOW * uiw);
static void destroy_windows(void);
static void disp_trans_table_choice(UIWINDOW uiwin, int row, int col, int indx);
static void display_status(String text);
static bool does_match(void *param, void *el);
static void edit_tt_menu(void);
#if !defined(DEADENDS)
static void edit_user_options(void);
static void edit_place_table(void);
#endif
static void end_action(void);
bool get_answer(UIWINDOW uiwin, int row, int col, String buffer, int buflen);
static int get_brwsmenu_size(int screen);
static RecordIndexEl *invoke_add_menu(void);
static void invoke_cset_display(void);
static void invoke_del_menu(void);
static int invoke_extra_menu(RecordIndexEl **rec);
static void invoke_utils_menu(void);
static void output_menu(UIWINDOW uiwin, DYNMENU dynmenu);
static void place_cursor_main(void);
static void place_std_msg(void);
static void platform_postcurses_init(void);
static void refresh_main(void);
static void register_screen_lang_callbacks(bool registering);
static void remove_uiwin(UIWINDOW uiwin);
static void repaint_add_menu(UIWINDOW uiwin);
static void repaint_delete_menu(UIWINDOW uiwin);
/*static void repaint_tt_menu(UIWINDOW uiwin);*/
static void repaint_utils_menu(UIWINDOW uiwin);
static void repaint_extra_menu(UIWINDOW uiwin);
static void repaint_main_menu(UIWINDOW uiwin);
static int resize_screen_impl(char * errmsg, int errsize);
static void run_report(bool picklist);
static void screen_on_lang_change(void *uparm);
static RecordIndexEl *search_for_one_record(void);
static void show_fam (UIWINDOW uiwin, RecordIndexEl *frec, int mode, int row, int hgt, int width, int * scroll, bool reuse);
bool show_record(UIWINDOW uiwin, CString key, int mode, LLRECT
	, int * scroll, bool reuse);
static void show_tandem_line(UIWINDOW uiwin, int row);
static void switch_to_uiwin(UIWINDOW uiwin);
static void touch_all(bool includeCurrent);
static void uicolor(UIWINDOW, LLRECT rect, char ch);
static int update_browse_menu(int screen);
static void update_screen_size(void);
//static bool yes_no_value(int c);

/*********************************************
 * local variables
 *********************************************/

/* what is showing now in status bar */
static char status_showing[150];
/* flag if it is not important to keep */
static bool status_transitory = false;


/* total screen lines used */
static int LINESTOTAL = LINESREQ;
/* number of lines for various menus */
static int EMPTY_MENU = -1; /* save one horizontal line */
/* the following values are default (larger screens get more) */
static int TANDEM_LINES_DEF = 6;     /* number of lines of tandem info */
static int AUX_LINES_DEF = 15;       /* number of lines in aux window */
/* working values */
static int TANDEM_LINES=0;
static int AUX_LINES=0;

int winx=0, winy=0; /* user specified window size */

static List *msg_list = 0;
static bool msg_flag = false; /* need to show msg list */
static bool viewing_msgs = false; /* user is viewing msgs */
static bool lock_std_msg = false; /* to hold status message */
static UIWINDOW active_uiwin = 0;
static List *list_uiwin = 0; /* list of all uiwindows */

/* we ought to use chtype, but only if it is typedef'd, but there is no
test to see if a type is typedef'd */
static llchtype gr_btee='+', gr_ltee='+', gr_rtee='+', gr_ttee='+';
static llchtype gr_hline='-', gr_vline= '|';
static llchtype gr_llx='*', gr_lrx='*', gr_ulx='*', gr_urx='*';

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*============================
 * set_screen_graphical -- Specify whether to use ncurses box characters
 *  graphical:   [IN]  whether to use ncurses graphical box lines
 *==========================*/
void
set_screen_graphical (bool graphical)
{
	if (graphical) {
		gr_btee = ACS_BTEE;
		gr_hline = ACS_HLINE;
		gr_ltee = ACS_LTEE;
		gr_rtee = ACS_RTEE;
		gr_ttee = ACS_TTEE;
		gr_vline = ACS_VLINE;
		gr_llx = ACS_LLCORNER;
		gr_lrx = ACS_LRCORNER;
		gr_ulx = ACS_ULCORNER;
		gr_urx = ACS_URCORNER;
	}
	else {
		gr_btee = '+';
		gr_hline = '-';
		gr_ltee = '+';
		gr_rtee = '+';
		gr_ttee = '+';
		gr_vline = '|';
		gr_llx = '*';
		gr_lrx = '*';
		gr_ulx = '*';
		gr_urx = '*';
	}
	
}
/*============================
 * init_screen -- Init screens
 *  returns 0 if current terminal is not large enough, or size requested too small
  *  errmsg:      [OUT] buffer to hold failure message
 *  errsize:     [IN]  size of errmsg buffer
 *==========================*/
int
init_screen (char * errmsg, int errsize)
{
	int rtn = resize_screen_impl(errmsg, errsize);
	if (rtn) { /* success */
		register_screen_lang_callbacks(true);
		platform_postcurses_init();
	}
	return rtn;
}
/*============================
 * update_screen_size -- Recreate windows etc if screen size has changed
 *==========================*/
static void
update_screen_size (void)
{
	char errmsg[512];
	resize_screen_impl(errmsg, sizeof(errmsg)/sizeof(errmsg[0]));
}
/*============================
 * resize_screen_impl -- setup windows & things dependent on screen size
 *  returns 0 if current terminal is not large enough, or size requested too small
 *  errmsg:      [OUT] buffer to hold failure message
 *  errsize:     [IN]  size of errmsg buffer
 * Safe to be called over and over again
 * Doesn't do anything if screen size is unchanged from previous size, or if unsupported size
 *==========================*/
static int
resize_screen_impl (char * errmsg, int errsize)
{
	int extralines=0;
	int newlines=0, newcols=0;

	/* stop & restart ncurses to update LINES & COLS */
	if (ll_lines > 0) {
		/* need endwin before initscr below */
		endwin();
	}
	if (1) {
		WINDOW *win = initscr();
		if (!win) {
			snprintf(errmsg, errsize, _("initscr failed"));
			return 0; /* fail */
		}
		noecho();
		keypad(win, 1);
	}

	/* by default, use full screen (as many rows & cols as they have */
	newlines=LINES;
	newcols = COLS;

	/* if user specified window size, use it, but check its validity */
	if (winx) {
		newlines = winy;
		newcols = winx;
		if (newcols > COLS || newlines > LINES) {
			snprintf(errmsg, errsize
				, _("The requested window size (" FMT_INT "," FMT_INT ") is too large for your terminal (%d,%d).\n")
				, newcols, newlines, COLS, LINES);
			return 0; /* fail */
		}
	}
	/* check that terminal meet minimum requirements */
	if (newcols < COLSREQ || newlines < LINESREQ) {
		snprintf(errmsg, errsize
			, _("The requested window size (" FMT_INT "," FMT_INT ") is too small for DeadEnds (%d,%d).\n")
			, newcols, newlines, COLSREQ, LINESREQ);
		return 0; /* fail */
	}

	if (ll_lines == newlines && ll_cols == newcols) {
		/* screen size is already configured & hasn't changed */
		return 1; /* succeed */
	}

	
	ll_lines = newlines;
	ll_cols = newcols;

	extralines = ll_lines - LINESREQ;
	LINESTOTAL = ll_lines;

	/* initialize browse window heights to default */
	TANDEM_LINES = TANDEM_LINES_DEF;
	AUX_LINES = AUX_LINES_DEF;
	/* increase for larger screens */
	if(extralines > 0) {
		TANDEM_LINES = TANDEM_LINES_DEF + (extralines / 2);
		AUX_LINES = AUX_LINES_DEF + extralines;
	}
	listui_init_windows(extralines);
	create_windows();
	brwsmenu_initialize(ll_lines, ll_cols);

	
	return 1; /* succeed */
}
/*============================
 * term_screen -- Terminate screens
 *  complement of init_screen
 *==========================*/
void
term_screen (void)
{
	register_screen_lang_callbacks(false);
	menuitem_terminate();
	active_uiwin = 0;
	destroy_windows();

	/* must clear window size so resize_screen_impl will
	start from scratch, in case we're closing and then
	starting up again (ie, Q option from main menu) */
	ll_lines = -1;
	ll_cols = -1;
}
/*=======================================
 * draw_win_box -- wrapper for curses box
 *  handles the case that user didn't want
 *  curses graphics
 *=====================================*/
void
draw_win_box (WINDOW * win)
{
	wborder(win, gr_vline, gr_vline, gr_hline, gr_hline, gr_ulx, gr_urx, gr_llx, gr_lrx);
}
/*=======================================
 * repaint_main_menu -- Display choices for main menu
 *  See function main_menu for actions
 *=====================================*/
static void
repaint_main_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	int row;
	char title[80];
	int width=sizeof(title);
	String str;

	uierase(uiwin);
	draw_win_box(win);
	show_horz_line(uiwin, 4, 0, ll_cols);
	show_horz_line(uiwin, ll_lines-3, 0, ll_cols);
	if (width > ll_cols-4)
		width = ll_cols-4;
	destrncpyf(title, width, uu8, _(qSmtitle), get_deadends_version(ll_cols-4));
	mvccwaddstr(win, 1, 2, title);
	mvccwaddstr(win, 2, 4, _(qScright));
	str = getdeoptint("FullDbPath", 1) ? database->filePath : database->lastSegment;
	mvccwprintw(win, 3, 4, _(qSdbname), str);
	row = 5;
	/* i18n problem: the letters are not being read from the menu strings */
	mvccwaddstr(win, row++, 2, _(qSplschs));
	mvccwaddstr(win, row++, 4, _(qSmn_mmbrws));
	mvccwaddstr(win, row++, 4, _(qSmn_mmsear));
	mvccwaddstr(win, row++, 4, _(qSmn_mmadd));
	mvccwaddstr(win, row++, 4, _(qSmn_mmdel));
	mvccwaddstr(win, row++, 4, _(qSmn_mmprpt));
	mvccwaddstr(win, row++, 4, _(qSmn_mmrpt));
	mvccwaddstr(win, row++, 4, _(qSmn_mmtt));
	mvccwaddstr(win, row++, 4, _(qSmn_mmut));
	mvccwaddstr(win, row++, 4, _(qSmn_mmex));
	mvccwaddstr(win, row++, 4, _(qSmn_changedb));
	mvccwaddstr(win, row++, 4, _(qSmn_exit));
}
/*==========================================
 * create_uiwindow_impl -- Create (or update) the WINDOW wrapper
 *  safe to call on existing UIWINDOW
 *========================================*/
static void
create_uiwindow_impl (UIWINDOW * puiw, CString name, WINDOW * win, int rows, int cols)
{
	UIWINDOW uiwin=0;
	ASSERT(puiw);
	uiwin = *puiw;
	if (!uiwin) {
		*puiw = uiwin = (UIWINDOW)stdalloc(sizeof(*uiwin));
		memset(uiwin, 0, sizeof(*uiwin));
		add_uiwin(uiwin);
	}
	if (uiwin->name)
		stdfree((String)uiwin->name);
	uiwin->name = strsave(name);
	if (uiw_win(uiwin) != win) {
		if (uiw_win(uiwin))
			delwin(uiw_win(uiwin));
		uiw_win(uiwin) = win;
	}
	uiw_rows(uiwin) = rows;
	uiw_cols(uiwin) = cols;
}
/*==========================================
 * add_uiwin -- Record new uiwin into master list
 *========================================*/
static void
add_uiwin (UIWINDOW uiwin)
{
	if (!list_uiwin)
		list_uiwin = createList(NULL, NULL, delete_uiwindow_impl, false);
	enqueue_list(list_uiwin, uiwin);
}
/*==========================================
 * remove_uiwin -- Remove uiwin from master list
 *========================================*/
static void
remove_uiwin (UIWINDOW uiwin)
{
	ASSERT(list_uiwin);
#if defined(DEADENDS)
	int index;
	if (searchList (list_uiwin, uiwin, &index)) {
	  removeFromList (list_uiwin, index);
	}
#else
	void *param = uiwin;
	bool deleteall = false;
	find_delete_list_elements(list_uiwin, param, &does_match, deleteall);
#endif
}
/*==========================================
 * does_match -- Used as callback to remove_uiwin
 *  in finding an element in a list
 *========================================*/
static bool
does_match (void *param, void *el)
{
	return param == el;
}
/*==========================================
 * create_boxed_newwin2 -- Create a window with
 *  an auxiliary box window outside it
 *========================================*/
static void
create_boxed_newwin2 (UIWINDOW * puiw, CString name, int rows, int cols)
{
	int begy = (LINES - rows)/2;
	int begx = (COLS - cols)/2;
	WINDOW * boxwin = newwin(rows, cols, begy, begx);
	WINDOW * win=0;
	ASSERT(puiw);
	++begy;
	++begx;
	win = subwin(boxwin, rows-2, cols-2, begy, begx);
	create_uiwindow_impl(puiw, name, win, rows-2, cols-2);
	uiw_boxwin(*puiw) = boxwin;
}
/*==========================================
 * delete_uiwindow_impl -- Delete UIWINDOW contents
 * Used by delete_uiwindow and list element destructor
 *========================================*/
static void
delete_uiwindow_impl (UIWINDOW w)
{
	if (w) {
		// delete window (curses)
		ASSERT(uiw_win(w));
		delwin(uiw_win(w));
		// delete boxwin (curses)
		if (uiw_boxwin(w))
			delwin(uiw_boxwin(w));
		// delete window name
		ASSERT(w->name);
		stdfree((String)w->name);
		// delete window
		stdfree(w);
	}
}

/*==========================================
 * delete_uiwindow -- Delete UIWINDOW wrapper
 * Created: 2002/01/23
 *========================================*/
static void
delete_uiwindow (UIWINDOW * uiw)
{
	if (*uiw) {
		UIWINDOW w = *uiw;
		// remove window from master list
		remove_uiwin(w);
		// delete window
		delete_uiwindow_impl(w);
		// clear pointer
		*uiw = 0;
	}
}

/*==========================================
 * create_newwin -- Create our WINDOW wrapper
 * Create ncurses window of specified size & location
 *  and wrap it with a WUIWINDOW wrapper
 *  and return that
 *========================================*/
static void
create_newwin (UIWINDOW * puiw, CString name, int rows, int cols, int begy, int begx)
{
	WINDOW * win = newwin(rows, cols, begy, begx);
	create_uiwindow_impl(puiw, name, win, rows, cols);
}
/*==========================================
 * create_newwin2 -- Create our WINDOW wrapper
 * Create ncurses window of specified size, centered on current physical screen
 *  and wrap it with a WUIWINDOW wrapper
 *  and return that
 *========================================*/
void
create_newwin2 (UIWINDOW * puiw, CString name, int rows, int cols)
{
	/* NEWWIN centers window on current physical screen */
	WINDOW * win = NEWWIN(rows, cols);
	create_uiwindow_impl(puiw, name, win,rows,cols);
}
/*==========================================
 * create_uisubwindow -- Create our WINDOW wrapper
 *  for a true (& permanent) subwindow
 *========================================*/
static void
create_uisubwindow (UIWINDOW * puiw, CString name, UIWINDOW parent
	, int rows, int cols
	, int begy, int begx)
{
	WINDOW * win = subwin(uiw_win(parent), rows, cols, begy, begx);
	create_uiwindow_impl(puiw, name, win, rows, cols);
	uiw_parent(*puiw) = parent;
	uiw_permsub(*puiw) = true;
}
/*==========================================
 * destroy_windows -- Undo create_windows
 *========================================*/
static void
destroy_windows (void)
{
	// As all windows are created via create_uiwindow_impl(), they are
	// added to the global window list in add_uiwin().
	//
	// Since list_uiwin was created via create_list3() and has an element
	// destructor, we can just call destroy_list(), which will remove each
	// element from the list, destroy it, and then finally destroy the
	// empty list itself.

	destroy_list(list_uiwin);
}
/*==========================================
 * create_windows -- Create and init windows
 *========================================*/
static void
create_windows (void)
{
	int col;
	
	create_boxed_newwin2(&stdout_win, "stdout_win", ll_lines-4, ll_cols-4);
	scrollok(uiw_win(stdout_win), true);
	
	col = COLS/4;
	create_newwin(&debug_box_win, "debug_box", 8, ll_cols-col-2, 1, col);

	create_uisubwindow(&debug_win, "debug", debug_box_win, 6, ll_cols-col-4, 2, col+1);
	scrollok(uiw_win(debug_win), true);

	MAINWIN_WIDTH = ll_cols;
	create_newwin2(&main_win, "main", ll_lines, MAINWIN_WIDTH);

	create_newwin2(&tt_menu_win, "tt_menu", 12,MAINWIN_WIDTH-7);

	create_newwin2(&ask_win, "ask", 4, 73);

	create_newwin2(&ask_msg_win, "ask_msg", 5, 73);


	/* tt_menu_win is drawn dynamically */
	draw_win_box(uiw_win(ask_win));
	draw_win_box(uiw_win(ask_msg_win));
	draw_win_box(uiw_win(debug_box_win));
}
/*=================================
 * display_screen --
 * There are six screens that all use
 * the main_win. MAIN_SCREEN is the
 * intro/main menu. The other 6 are all
 * browse screens.
 * cur_screen tells which is active.
 *===============================*/
void
display_screen (int new_screen)
{
	UIWINDOW uiwin = main_win;
	WINDOW * win = uiw_win(uiwin);
	cur_screen = new_screen;
	check_stdout();
	if (!status_showing[0] || status_transitory)
		place_std_msg();
	else
		mvccwaddstr(win, ll_lines-2, 2, status_showing);
	place_cursor_main();
	switch_to_uiwin(uiwin);
}
/*=====================================
 * do_check_stdout -- Pause for stdout/err display
 *  if it is up
 *===================================*/
static int
do_prompt_stdout (CString prompt)
{
	int ch=0;
	llwprintf("\n%s\n", prompt);
	crmode();
	ch = wgetch(uiw_win(stdout_win));
	nocrmode();
	/* ok the status string was available until they struck a key */
	clear_status();
	return ch;
}
/*=====================================
 * check_stdout -- If stdout populated,
 *  prompt user to acknowledge, close & deactivate it
 *===================================*/
static void
check_stdout (void)
{
	if (active_uiwin == stdout_win) {
		if (stdout_vis) {
			do_prompt_stdout(_(qShitkey));
			stdout_vis = false;
		}
		deactivate_uiwin_and_touch_all();
	}
}
/*=====================================
 * prompt_stdout -- Prompt user in stdout
 * This is like check_stdout, except it always
 * prompts, and it returns the response char,
 * and it doesn't clear stdout
 *===================================*/
int
prompt_stdout (CString prompt)
{
	int i;
	if (active_uiwin != stdout_win)
		activate_uiwin(stdout_win);
	stdout_vis = true;
	i = do_prompt_stdout(prompt);
	return i;
}
/*=====================================
 * search_for_one_record -- Invoke search menu & trim to one record
 *===================================*/
static RecordIndexEl *
search_for_one_record (void)
{
	Sequence *seq = invoke_search_menu();
	if (!seq) return NULL;
	if (!lengthSequence(seq)) {
		remove_indiseq(seq);
		return NULL;
	}
	/* namesort uses canonkeysort for non-persons */
	namesort_indiseq(seq);
	return chooseFromSequence(seq, DOASK1,
		_("Search results"), _("Search results"));
}
/*=====================================
 * main_menu -- Handle main_menu screen
 *===================================*/
void
main_menu (void)
{
	int c;
	UIWINDOW uiwin = main_win;
	WINDOW * win = uiw_win(uiwin);
	repaint_main_menu(uiwin);
	display_screen(MAIN_SCREEN);
	c = interact_choice_string(uiwin, "bsadprtuxqQ");
	place_std_msg();
	wrefresh(win);
	switch (c) {
	case 'b': main_browse(NULL, BROWSE_INDI); break;
	case 's':
		{
			RecordIndexEl *rec = search_for_one_record();
			if (rec)
				main_browse(rec, BROWSE_UNK);
		}
		break;
	case 'a':
		{
			RecordIndexEl *rec = 0;

			rec = invoke_add_menu();
			if (rec)
				main_browse(rec, BROWSE_UNK);
		}
		break;
	case 'd':
		{
			invoke_del_menu();
		}
		break;
	case 'p': run_report(true); break;
	case 'r': run_report(false); break;
	case 't': edit_tt_menu(); break;
	case 'u': invoke_utils_menu(); break;
	case 'x':
		{
			RecordIndexEl *rec=0;
			c = invoke_extra_menu(&rec);
			if (c != BROWSE_QUIT)
				main_browse(rec, c);
			/* main_browse consumed rec */
		}
		break;
	case 'q': alldone = 1; break;
	case 'Q':
		uierase(main_win);
		alldone = 2;
		break;
	}
}
/*=========================================
 * run_report -- run a report program
 *  @picklist:  [IN]  display list of reports to user ?
 *=======================================*/
void
run_report (bool picklist)
{
	List *progfiles = NULL; /* will prompt for report */
	String ofile = NULL; /* will prompt for output file */
	bool timing = true;
	begin_action();
	interp_main(progfiles, ofile, picklist, timing);
	end_action(); /* displays any errors that happened */
}
/*=========================================
 * update_browse_menu -- redraw menu if needed
 *  This is browse menu using dynamic menu
 *  in rectangle at bottom of screen
 * Returns number lines used by menu
 *=======================================*/
static int
update_browse_menu (int screen)
{
	DYNMENU dynmenu = get_screen_dynmenu(screen);
	int lines = LINESTOTAL-OVERHEAD_MENU - get_brwsmenu_size(screen);
	if (dynmenu->dirty || (cur_screen != screen)) {
		UIWINDOW uiwin = main_win;
		WINDOW *win = uiw_win(uiwin);
		uierase(uiwin);
		draw_win_box(win);
		show_horz_line(uiwin, ll_lines-3,  0, ll_cols);
		if (!dynmenu->hidden) {
			/* display title */
			int width = ll_cols;
			char prompt[128];
			check_menu(dynmenu);
			/* display prompt immediately above menu */
			destrncpy(prompt, _(qSplschs), sizeof(prompt), uu8);
			dynmenu->cur_x = strlen(_(qSplschs))+3;
			dynmenu->cur_y = dynmenu->top - 1;
			destrapps(prompt, sizeof(prompt), uu8, "             ");
			destrappf(prompt, sizeof(prompt), uu8, _("(pg " FMT_INT "/" FMT_INT ")")
				, dynmenu->page+1, dynmenu->pages);
			/* display line across */
			show_horz_line(uiwin, dynmenu->top-2, 0, width);
			/* display prompt */
			mvccwaddstr(win, dynmenu->top-1, dynmenu->left-1, prompt);
			/* draw menu */
			output_menu(uiwin, dynmenu);
		}
	}
	dynmenu->dirty = false;
	return lines;
}
/*=========================================
 * show_indi -- Show indi according to mode
 *  @uiwin:  [IN]  where to display
 *  @indi:   [IN]  whom to display
 *  @mode:   [IN]  how to display (eg, traditional, gedcom, ...)
 *  @rect:   [IN]  rectangular area in which to display
 *  @scroll: [I/O] how far down display is scrolled
 *  @reuse:  [IN]  flag to save recalculating display strings
 *=======================================*/
void
show_indi (UIWINDOW uiwin, RecordIndexEl *irec, int mode, LLRECT rect
	, int * scroll, bool reuse)
{
	if (mode=='g')
		show_gedcom(uiwin, irec, GDVW_NORMAL, rect, scroll, reuse);
	else if (mode=='x')
		show_gedcom(uiwin, irec, GDVW_EXPANDED, rect, scroll, reuse);
	else if (mode=='t')
		show_gedcom(uiwin, irec, GDVW_TEXT, rect, scroll, reuse);
	else if (mode=='a')
		show_ancestors(uiwin, irec, rect, scroll, reuse);
	else if (mode=='d')
		show_descendants(uiwin, irec, rect, scroll, reuse);
	else
		show_indi_vitals(uiwin, irec, rect, scroll, reuse);
}
/*=========================================
 * show_fam -- Show family
 *  fam:   [IN]  whom to display
 *  mode:  [IN]  how to display
 *  row:   [IN]  starting row to use
 *  hgt:   [IN]  how many rows allowed
 *  width: [IN]  how many columns allowed
 *  reuse: [IN]  flag to save recalculating display strings
 *=======================================*/
static void
show_fam (UIWINDOW uiwin, RecordIndexEl *frec, int mode, int row, int hgt
	, int width, int * scroll, bool reuse)
{
	struct tag_llrect rect;
	rect.top = row;
	rect.bottom = row+hgt-1;
	rect.left = 1;
	rect.right = width-1;
	if (mode=='g')
		show_gedcom(uiwin, frec, GDVW_NORMAL, &rect, scroll, reuse);
	else if (mode=='x')
		show_gedcom(uiwin, frec, GDVW_EXPANDED, &rect, scroll, reuse);
	else if (mode=='t')
		show_gedcom(uiwin, frec, GDVW_TEXT, &rect, scroll, reuse);
	else
		show_fam_vitals(uiwin, frec, row, hgt, width, scroll, reuse);
}
/*=========================================
 * display_indi -- Paint indi on-screen
 *=======================================*/
void
display_indi (RecordIndexEl *indi, int mode, bool reuse)
{
	int screen = ONE_PER_SCREEN;
	int lines=0;
	struct tag_llrect rect;
	update_screen_size(); /* ensure screen size is current */
	lines = update_browse_menu(screen);
	/* leave room for box all around */
	rect.top = 1;
	rect.bottom = lines;
	rect.left = 1;
	rect.right = MAINWIN_WIDTH-2;
	show_indi(main_win, indi, mode, &rect, &Scroll1, reuse);
	display_screen(screen);
}
/*=========================================
 * interact_indi -- Get menu choice for indi browse
 *=======================================*/
int
interact_indi (void)
{
	return interact_screen_menu(main_win, ONE_PER_SCREEN);
}
/*=======================================
 * display_fam -- Paint fam on-screen
 *=====================================*/
void
display_fam (RecordIndexEl *frec, int mode, bool reuse)
{
	int width=0;
	int screen = ONE_FAM_SCREEN;
	int lines=0;
	update_screen_size(); /* ensure screen size is current */
	lines = update_browse_menu(screen);
	width = MAINWIN_WIDTH;
	show_fam(main_win, frec, mode, 1, lines, width, &Scroll1, reuse);
	display_screen(screen);
}
/*=========================================
 * interact_fam -- Get menu choice for indi browse
 *=======================================*/
int
interact_fam (void)
{
	return interact_screen_menu(main_win, ONE_FAM_SCREEN);
}
/*=============================================
 * display_2indi -- Paint tandem indi screen
 *===========================================*/
void
display_2indi (RecordIndexEl *irec1, RecordIndexEl *irec2, int mode)
{
	int screen = TWO_PER_SCREEN;
	int lines=0;
	int lines1=0,lines2=0;
	bool reuse = false; /* can't reuse display strings in tandem */
	struct tag_llrect rect;

	update_screen_size(); /* ensure screen size is current */
	
	lines = update_browse_menu(screen);
	lines--; /* for tandem line */
	lines2 = lines/2;
	lines1 = lines - lines2;

	rect.top = 1;
	rect.bottom = lines1;
	rect.left = 1;
	rect.right = MAINWIN_WIDTH-2;

	show_indi(main_win, irec1, mode, &rect, &Scroll1, reuse);
	show_tandem_line(main_win, lines1+1);
	switch_scrolls();
	rect.top = lines1+2;
	rect.bottom = lines+1;
	show_indi(main_win, irec2, mode, &rect, &Scroll1, reuse);
	switch_scrolls();

	display_screen(screen);
}
/*=========================================
 * interact_2indi -- Get menu choice for tandem indi
 *=======================================*/
int
interact_2indi (void)
{
	return interact_screen_menu(main_win, TWO_PER_SCREEN);
}
/*====================================
 * show_tandem_line -- Display horizontal line between top & bottom
 * PR 1999/03
 *==================================*/
static void
show_tandem_line (UIWINDOW win, int row)
{
	show_horz_line(win, row, 0, ll_cols);
}
/*=============================================
 * display_2fam -- Paint tandem families
 *===========================================*/
void
display_2fam (RecordIndexEl *frec1, RecordIndexEl *frec2, int mode)
{
	UIWINDOW uiwin = main_win;
	int width=0;
	int screen = TWO_FAM_SCREEN;
	int lines=0;
	int lines1=0,lines2=0;
	bool reuse = false; /* can't reuse display strings in tandem */

	update_screen_size(); /* ensure screen size is current */
	width=MAINWIN_WIDTH;

	lines = update_browse_menu(screen);
	lines--; /* for tandem line */
	lines2 = lines/2;
	lines1 = lines - lines2;

	show_fam(uiwin, frec1, mode, 1, lines1, width, &Scroll1, reuse);
	show_tandem_line(main_win, lines1+1);
	switch_scrolls();
	show_fam(uiwin, frec2, mode, lines1+2, lines2, width, &Scroll1, reuse);
	switch_scrolls();

	display_screen(screen);
}
/*=========================================
 * interact_2fam -- Get menu choice for tandem fam
 *=======================================*/
int
interact_2fam (void)
{
	return interact_screen_menu(main_win, TWO_FAM_SCREEN);
}
/*=========================================
 * interact_popup -- Get menu choice for a popup window
 *=======================================*/
int
interact_popup (UIWINDOW uiwin, String str)
{
	return interact_choice_string(uiwin, str);
}
/*=======================================
 * aux_browse -- Handle aux_browse screen
 * This is used for browsing S, E, or X records.
 *=====================================*/
int
aux_browse (RecordIndexEl *rec, int mode, bool reuse)
{
	UIWINDOW uiwin = main_win;
	int lines = update_browse_menu(AUX_SCREEN);
	struct tag_llrect rect;
	rect.top = 1;
	rect.bottom = lines;
	rect.left = 1;
	rect.right = MAINWIN_WIDTH-1;
	show_aux(uiwin, rec, mode, &rect,  &Scroll1, reuse);
	display_screen(AUX_SCREEN);
	return interact_screen_menu(uiwin, AUX_SCREEN);
}
/*=========================================
 * list_browse -- Handle list_browse screen
 *  cur is passed for GUI doing
 *  direct navigation in list
 *  this curses implementation does not use them
 *=======================================*/
int
list_browse (Sequence *seq, int top, int * cur, int mark)
{
	if (cur_screen != LIST_SCREEN) paint_list_screen();
	show_big_list(seq, top, *cur, mark);
	display_screen(LIST_SCREEN);
	return interact_choice_string(main_win, "jkeimrtbanx$^udUDq");
}

/*======================================
 * ask_for_input_filename_impl -- Ask user for a filename
 *  (in curses version, we don't differentiate input from output prompts)
 *  ttl1:    [IN]  title of question (1rst line)
 *  path:    [IN]  path prompt (2nd line)
 *  prmpt:   [IN]  prompt of question (3rd line)
 *  buffer:  [OUT] response
 *  buflen:  [IN]  max size of response
 *====================================*/
bool
ask_for_filename_impl (CString ttl, CString path, CString prmpt, String buffer, int buflen)
{
	/* display current path (truncated to fit) */
	char curpath[120];
	int len = sizeof(curpath);
	if (len > uiw_cols(ask_msg_win)-2)
		len = uiw_cols(ask_msg_win)-2;
	curpath[0] = 0;
	destrapps(curpath, len, uu8, _(qSiddefpath));
	destrapps(curpath, len, uu8, compressPath(path, len-strlen(curpath)-1));

	return ask_for_string2(ttl, curpath, prmpt, buffer, buflen);
}
/*======================================
 * refresh_main -- touch & refresh main or stdout
 *  as appropriate
 *====================================*/
static void
refresh_main (void)
{
	WINDOW *win = stdout_vis ? uiw_win(stdout_win) : uiw_win(main_win);
	wrefresh(win);
}
/*======================================
 * ask_for_string -- Ask user for string
 *  returns static buffer
 *  ttl:     [IN]  title of question (1rst line)
 *  prmpt:   [IN]  prompt of question (2nd line)
 *  buffer:  [OUT] response
 *  buflen:  [IN]  max size of response
 *====================================*/
bool
ask_for_string (CString ttl, CString prmpt, String buffer, int buflen)
{
	UIWINDOW uiwin = ask_win;
	WINDOW *win = uiw_win(uiwin);
	bool rtn;
	uierase(uiwin);
	draw_win_box(win);
	mvccuwaddstr(uiwin, 1, 1, ttl);
	mvccuwaddstr(uiwin, 2, 1, prmpt);
	activate_uiwin(uiwin);
	rtn = get_answer(uiwin, 2, strlen(prmpt) + 2, buffer, buflen);
	deactivate_uiwin_and_touch_all();
	return rtn;
}
/*======================================
 * ask_for_string2 -- Ask user for string
 * Two lines of title
 *  returns static buffer
 *  ttl1:    [IN]  title of question (1rst line)
 *  ttl2:    [IN]  2nd line of title
 *  prmpt:   [IN]  prompt of question (3rd line)
 *  buffer:  [OUT] response
 *  buflen:  [IN]  max size of response
 *====================================*/
bool
ask_for_string2 (CString ttl1, CString ttl2, CString prmpt, String buffer, int buflen)
{
	UIWINDOW uiwin = ask_msg_win;
	WINDOW *win = uiw_win(uiwin);
	bool rtn;
	uierase(uiwin);
	draw_win_box(win);
	mvccwaddstr(win, 1, 1, ttl1);
	mvccwaddstr(win, 2, 1, ttl2);
	mvccwaddstr(win, 3, 1, prmpt);
	wrefresh(win);
	activate_uiwin(uiwin);
	rtn = get_answer(uiwin, 3, strlen(prmpt) + 2, buffer, buflen);
	deactivate_uiwin_and_touch_all();
	return rtn;
}

/*===========================================
 * ask_for_char_msg -- Ask user for character
 *  msg:   [IN]  top line displayed (optional)
 *  ttl:   [IN]  2nd line displayed
 *  prmpt: [IN]  3rd line text before cursor
 *  ptrn:  [IN]  List of allowable character responses
 *=========================================*/
int
ask_for_char_msg (CString msg, CString ttl, CString prmpt, CString ptrn)
{
	UIWINDOW uiwin = (msg ? ask_msg_win : ask_win);
	WINDOW *win = uiw_win(uiwin);
	int y;
	int rv;
	uierase(uiwin);
	draw_win_box(win);
	y = 1;
	if (msg)
		mvccwaddstr(win, y++, 2, msg);
	mvccwaddstr(win, y++, 2, ttl);
	mvccwaddstr(win, y++, 2, prmpt);
	wrefresh(win);
	rv = interact_choice_string(uiwin, ptrn);
	return rv;
}
/*============================================
 * chooseFromArray -- Choose from string list
 *  ttl:      [IN] title for choice display
 *  no:       [IN] number of choices
 *  pstrngs:  [IN] array of choices
 * returns 0-based index chosen, or -1 if cancelled
 *==========================================*/
int
chooseFromArray (CString ttl, int no, String *pstrngs)
{
	bool selecting = true;
	if (!ttl) ttl=_(qSdefttl);
	return chooseOrViewArray(ttl, no, pstrngs, selecting, 0, 0);
}
/*============================================
 * display_list -- Show user list of information
 *  ttl:    [IN] title for display
 *  list    [IN] list of string entries
 * returns 0-based index chosen, or -1 if cancelled
 *==========================================*/
int
display_list (CString ttl, List *list)
{
	/* TODO: Need to set some flag to suppress i & <enter> */
	return chooseFromList(ttl, list);
}

/*============================================
 * chooseFromArray_x -- Choose from string list
 *  ttl:      [IN]  title for choice display
 *  no:       [IN]  number of choices
 *  pstrngs:  [IN]  array of choices
 *  detfnc:   [IN]  callback for details about items
 *  param:    [IN]  opaque type for callback
 * returns 0-based index chosen, or -1 if cancelled
 *==========================================*/
int
chooseFromArray_x (CString ttl, int no, String *pstrngs, DETAILFNC detfnc
	, void *param)
{
	bool selecting = true;
	if (!ttl) ttl=_(qSdefttl);
	return chooseOrViewArray(ttl, no, pstrngs, selecting, detfnc, param);
}
/*============================================
 * view_array -- Choose from string list
 *  ttl:      [IN] title for choice display
 *  no:       [IN] number of choices
 *  pstrngs:  [IN] array of choices
 * returns 0-based index chosen, or -1 if cancelled
 *==========================================*/
void
view_array (CString ttl, int no, String *pstrngs)
{
	bool selecting = false;
	chooseOrViewArray(ttl, no, pstrngs, selecting, 0, 0);
}
/*============================================
 * chooseOrViewArray -- Implement choose/view from array
 *  ttl:       [IN]  title for choice display
 *  no:        [IN]  number of choices
 *  pstrngs:   [IN]  array of choices
 *  selecting: [IN]  if FALSE then view-only
 *  detfnc:    [IN]  callback for details about items
 *  param:     [IN]  opaque type for callback
 * returns 0-based index chosen, or -1 if cancelled
 *==========================================*/
static int
chooseOrViewArray (CString ttl, int no, String *pstrngs, bool selecting
	, DETAILFNC detfnc, void *param)
{
	int rv;
	rv = array_interact(ttl, no, pstrngs, selecting, detfnc, param);
	refresh_main();
	return rv;
}

/*==============================
 * draw_tt_win -- Draw menu for edit translations
 *============================*/
static void
draw_tt_win (String prompt)
{
	UIWINDOW uiwin = tt_menu_win;
	WINDOW *win = uiw_win(uiwin);
	int row = 0;
	uierase(uiwin);
	draw_win_box(win);
	row = 1;
	mvccwaddstr(win, row++, 2, prompt);
	disp_trans_table_choice(uiwin, row++, 4, MEDIN);
	disp_trans_table_choice(uiwin, row++, 4, MINED);
	disp_trans_table_choice(uiwin, row++, 4, MGDIN);
	disp_trans_table_choice(uiwin, row++, 4, MINGD);
	disp_trans_table_choice(uiwin, row++, 4, MDSIN);
	disp_trans_table_choice(uiwin, row++, 4, MINDS);
	disp_trans_table_choice(uiwin, row++, 4, MRPIN);
	disp_trans_table_choice(uiwin, row++, 4, MINRP);
	mvccwaddstr(win, row++, 4, _(qSmn_ret));
}
/*==============================
 * disp_trans_table_choice -- Display line in
 * translation table menu, & show current info
 * Created: 2001/07/20
 *============================*/
static void
disp_trans_table_choice (UIWINDOW uiwin, int row, int col, int trnum)
{
	XLAT xlat;
	ZSTR zstr, z1;

	xlat = transl_get_predefined_xlat(trnum);
	zstr = transl_get_predefined_menukey(trnum);
	z1 = transl_get_predefined_name(trnum);
	zs_apps(zstr, " ");
	zs_appz(zstr, z1);
	zs_apps(zstr, " : ");
	if (xlat) {
		zs_appz(zstr, transl_get_description(xlat));
	} else {
		zs_sets(zstr, _("No conversion"));
	}
	mvccuwaddstr(uiwin, row, col, zs_str(zstr));
	zs_free(&zstr);
}
/*============================
 * invoke_add_menu -- Handle add menu
 * returns addref'd record
 *==========================*/
static RecordIndexEl *
invoke_add_menu (void)
{
	UIWINDOW uiwin=0;
	WINDOW * win=0;
	RecordIndexEl *rec=0;
	int code;

	if (!add_menu_win) {
		create_newwin2(&add_menu_win, "add_menu", 8, 66);
		add_menu_win->outdated = true; /* needs drawing */
	}

	if (add_menu_win->outdated) {
		repaint_add_menu(add_menu_win);
	}
	uiwin = add_menu_win;
	win = uiw_win(uiwin);

	activate_uiwin(uiwin);
	wmove(win, 1, 27);
	code = interact_choice_string(uiwin, "pfcsq");
	deactivate_uiwin_and_touch_all();

	switch (code) {
	case 'p':
		rec = add_indi_by_edit(false);
		break;
	case 'f': add_family_by_edit(NULL, NULL, NULL, false); break;
	case 'c': my_prompt_add_child(NULL, NULL); break;
	case 's': prompt_add_spouse(NULL, NULL, true); break;
	case 'q': break;
	}
	return rec;
}
/*===============================
 * invoke_del_menu -- Handle delete menu
 *=============================*/
void
invoke_del_menu (void)
{
	int code;
	UIWINDOW uiwin=0;
	WINDOW * win=0;
	if (!del_menu_win) {
		create_newwin2(&del_menu_win, "del_menu", 9, 66);
		del_menu_win->outdated = true; /* needs drawing */
	}
	if (del_menu_win->outdated) {
		repaint_delete_menu(del_menu_win);
	}
	uiwin = del_menu_win;
	win = uiw_win(uiwin);

	activate_uiwin(uiwin);
	wmove(win, 1, 30);
	code = interact_choice_string(uiwin, "csifoq");
	deactivate_uiwin_and_touch_all();

	switch (code) {
	case 'c': chooseAndRemoveChild(NULL, NULL, false); break;
	case 's': chooseAndRemoveSpouse(NULL, NULL, false); break;
	case 'i': chooseAndRemovePerson(NULL, DOCONFIRM); break;
	case 'f': chooseAndRemoveFamily(); break;
	case 'o': chooseAndRemoveAnyRecord(NULL, DOCONFIRM); break;
	case 'q': break;
	}
}
/*======================================
 * invoke_cset_display -- Handle character set menu
 *====================================*/
static void
invoke_cset_display (void)
{
	List *list = createList (NULL, NULL, free, false);
	ZSTR zstr=zs_newn(80);

	zs_setf(zstr, "%s: %s", _("Internal codeset"), int_codeset);
	enqueue_list(list, strsave(zs_str(zstr)));
	if (uu8) {
		enqueue_list(list, strsave(_("Internal UTF-8: Yes")));
	} else {
		enqueue_list(list, strsave(_("Internal UTF-8: No")));
	}

	if (are_locales_supported())
		enqueue_list(list, strsave(_("Locales are enabled.")));
	else
		enqueue_list(list, strsave(_("Locales are disabled.")));
	
	if (is_nls_supported()) {
		enqueue_list(list, strsave(_("NLS (National Language Support) is compiled in.")));
		zs_setf(zstr, "LocaleDir (default): %s", LOCALEDIR);
		enqueue_list(list, strsave(zs_str(zstr)));
		zs_setf(zstr,  "LocaleDir (override): %s", getdeoptstr("LocaleDir", ""));
		enqueue_list(list, strsave(zs_str(zstr)));
	} else {
		enqueue_list(list, strsave(_("NLS (National Language Support) is not compiled in.")));
	}

	if (1) {
		CString str = get_gettext_codeset();
		str = str ? str : "";
		zs_setf(zstr, "bind_textdomain_codeset: %s", str);
		enqueue_list(list, strsave(zs_str(zstr)));
	}

	add_shims_info(list);

	if (is_iconv_supported())
		enqueue_list(list, strsave(_("iconv (codeset conversion) is compiled in.")));
	else
		enqueue_list(list, strsave(_("iconv (codeset conversion) is not compiled in.")));
	
	zs_setf(zstr, _("Startup collate locale: %s"), get_original_locale_collate());
	enqueue_list(list, strsave(zs_str(zstr)));
	
	zs_setf(zstr, _("Startup messages locale: %s")	, get_original_locale_msgs());
	enqueue_list(list, strsave(zs_str(zstr)));
	
	zs_setf(zstr, _("Current collate locale: %s"), get_current_locale_collate());
	enqueue_list(list, strsave(zs_str(zstr)));
	
	zs_setf(zstr, _("Current messages locale: %s"), get_current_locale_msgs());
	enqueue_list(list, strsave(zs_str(zstr)));

	zs_setf(zstr, _("Collation routine: %s"), ll_what_collation());
	enqueue_list(list, strsave(zs_str(zstr)));
	
	if (eqstr_ex(gui_codeset_in, gui_codeset_out)) {
		zs_setf(zstr, _("GUI codeset: %s"), gui_codeset_in);
		enqueue_list(list, strsave(zs_str(zstr)));
	} else {
		zs_setf(zstr, _("GUI output codeset: %s"), gui_codeset_out);
		enqueue_list(list, strsave(zs_str(zstr)));
		zs_setf(zstr, _("GUI input codeset: %s"), gui_codeset_in);
		enqueue_list(list, strsave(zs_str(zstr)));
	}

	if (eqstr_ex(editor_codeset_in, editor_codeset_out)) {
		zs_setf(zstr, _("editor codeset: %s"), editor_codeset_in);
		enqueue_list(list, strsave(zs_str(zstr)));
	} else {
		zs_setf(zstr, _("editor output codeset: %s"), editor_codeset_out);
		enqueue_list(list, strsave(zs_str(zstr)));
		zs_setf(zstr, _("editor input codeset: %s"), editor_codeset_in);
		enqueue_list(list, strsave(zs_str(zstr)));
	}

	if (eqstr_ex(report_codeset_in, report_codeset_out)) {
		zs_setf(zstr, _("report codeset: %s"), report_codeset_in);
		enqueue_list(list, strsave(zs_str(zstr)));
	} else {
		zs_setf(zstr, _("report output codeset: %s"), report_codeset_out);
		enqueue_list(list, strsave(zs_str(zstr)));
		zs_setf(zstr, _("report input codeset: %s"), report_codeset_in);
		enqueue_list(list, strsave(zs_str(zstr)));
	}

	if (eqstr_ex(gedcom_codeset_in, gedcom_codeset_out)) {
		zs_setf(zstr, _("GEDCOM codeset: %s"), gedcom_codeset_in);
		enqueue_list(list, strsave(zs_str(zstr)));
	} else {
		zs_setf(zstr, _("gedcom output codeset: %s"), gedcom_codeset_out);
		enqueue_list(list, strsave(zs_str(zstr)));
		zs_setf(zstr, _("gedcom input codeset: %s"), gedcom_codeset_in);
		enqueue_list(list, strsave(zs_str(zstr)));
	}

	zs_setf(zstr, "TTPATH: %s", getdeoptstr("TTPATH", "."));
	enqueue_list(list, strsave(zs_str(zstr)));

	if (charprops_is_loaded()) {
		enqueue_list(list, strsave(_("UTF-8 charprops loaded" )));
	} else {
		enqueue_list(list, strsave(_("UTF-8 charprops not loaded" )));
	}


	display_list(_("Codeset information"), list);
	destroy_list(list);
	zs_free(&zstr);
}
/*======================================
 * add_shims_info -- Add information about gettext and iconv dlls
 *====================================*/
static void
#if defined WIN32_INTL_SHIM || defined WIN32_ICONV_SHIM
add_shims_info (List *list)
#else
add_shims_info (ATTRIBUTE_UNUSED List *list)
#endif
{
#if defined WIN32_INTL_SHIM || defined WIN32_ICONV_SHIM
	ZSTR zstr=zs_newn(80);
#if defined WIN32_INTL_SHIM
	{
		char value[MAXPATHLEN];
		if (intlshim_get_property("dll_path", value, sizeof(value)))
		{
			zs_setf(zstr, _("gettext dll: %s"), value);
			enqueue_list(list, strsave(zs_str(zstr)));
			if (intlshim_get_property("dll_version", value, sizeof(value)))
			{
				zs_setf(zstr, _("gettext dll version: %s"), value);
				enqueue_list(list, strsave(zs_str(zstr)));
			}
			else
			{
				enqueue_list(list, strsave(_("gettext dll had no version")));
			}
		}
		else
		{
			enqueue_list(list, strsave(_("no gettext dll found")));
		}
	}
#endif
#if defined WIN32_ICONV_SHIM
	{
		char value[MAXPATHLEN];
		if (iconvshim_get_property("dll_path", value, sizeof(value)))
		{
			zs_setf(zstr, _("iconv dll: %s"), value);
			enqueue_list(list, strsave(zs_str(zstr)));
			if (iconvshim_get_property("dll_version", value, sizeof(value)))
			{
				zs_setf(zstr, _("iconv dll version: %s"), value);
				enqueue_list(list, strsave(zs_str(zstr)));
			}
			else
			{
				enqueue_list(list, strsave(_("iconv dll had no version")));
			}
		}
		else
		{
			enqueue_list(list, strsave(_("no iconv dll found")));
		}
	}
#endif
	zs_free(&zstr);
#endif
}
/*======================================
 * invoke_trans_menu -- menu for translation tables
 * TODO: decide whether to bring this back or not
 *====================================*/
#ifdef UNUSED_CODE
static void
invoke_trans_menu (void)
{
	int code;
	UIWINDOW uiwin=0;
	bool done=false;

	if (!trans_menu_win) {
		create_newwin2(&trans_menu_win,"trans_menu",10,66);
	}
	uiwin = trans_menu_win;

	while (!done) {
		stdout_vis=false;
		repaint_trans_menu(uiwin);
		reactivate_uiwin(uiwin);
		wmove(uiw_win(uiwin), 1, strlen(_(qSmn_tt_ttl))+3);
		code = interact(uiwin, "elsxiq", -1);

		begin_action();
		switch (code) {
		case 'e': edit_tt_menu(); break;
		case 'l': load_tt_action(); break;
		case 's': save_tt_action(); break;
		case 'x': export_tts(); break;
		case 'i': import_tts(); break;
		case 'q': done=true; break;
		}
		end_action(); /* displays any errors that happened */
	}
	deactivate_uiwin_and_touch_all();
}
#endif
/*======================================
 * edit_tt_menu -- menu for "Edit translation table"
 *====================================*/
static void
edit_tt_menu (void)
{
	int ttnum;
	while ((ttnum = choose_tt(_(qSmn_edttttl))) != -1) {
		edit_mapping(ttnum);
		stdout_vis = false; /* don't need to see errors after done */
	}
}
/*======================================
 * load_tt_action -- menu for "Load translation table"
 *====================================*/
#ifdef UNUSED_CODE
static void
load_tt_action (void)
{
	FILE * fp;
	String fname=0;
	int ttnum;
	String ttimportdir;

	/* Ask which table */
	ttnum = choose_tt(_(qSmn_svttttl));
	if (ttnum == -1) return;
	if (ttnum < 0 || ttnum >= NUM_TT_MAPS) {
		msg_error("%s", _(qSbadttnum));
		return;
	}

	/* Ask whence to load it */
	ttimportdir = getdeoptstr("TTPATH", ".");
	fp = ask_for_input_file(DEREADTEXT, _(qSmintt), &fname, ttimportdir, ".tt");
	if (fp) {
		fclose(fp);
		/* Load it */
		if (!load_new_tt(fname, ttnum))
			msg_error("%s", _(qSdataerr));
	}
	strfree(&fname);
}
/*======================================
 * save_tt_action -- save a translation table
 * to a file
 *====================================*/
static void
save_tt_action (void)
{
	FILE * fp;
	String fname=0;
	int ttnum;
	String ttexportdir;
	
	/* Ask which table */
	ttnum = choose_tt(_(qSmn_svttttl));
	if (ttnum == -1) return;
	if (ttnum < 0 || ttnum >= NUM_TT_MAPS) {
		msg_error("%s", _(qSbadttnum));
		return;
	}
	if (!transl_get_legacy_tt(ttnum)) {
		msg_error("%s", _(qSnosuchtt));
		return;
	}
	/* Ask whither to save it */
	ttexportdir = getdeoptstr("DETTEXPORT", ".");
	fp = ask_for_output_file(DEWRITETEXT, _(qSmouttt), &fname, ttexportdir, ".tt");
	if (fp) {
		fclose(fp);
		/* Save it */
		if (!save_tt_to_file(ttnum, fname)) {
			msg_error("%s", _(qSdataerr));
			strfree(&fname);
			return;
		}
	}
	strfree(&fname);
}
#endif
/*======================================
 * choose_tt -- select a translation table (-1 for none)
 *====================================*/
static int
choose_tt (String prompt)
{
	int code;
	UIWINDOW uiwin = tt_menu_win;
	while (1) {
		draw_tt_win(prompt);
		activate_uiwin(uiwin);
		wmove(uiw_win(uiwin), 1, strlen(prompt)+3);
		code = interact_choice_string(uiwin, "emixgdrq");
		deactivate_uiwin_and_touch_all();
		switch (code) {
		case 'e': return MEDIN;
		case 'm': return MINED;
		case 'i': return MGDIN;
		case 'x': return MINGD;
		case 'g': return MDSIN;
		case 'd': return MINDS;
		case 'r': return MINRP;
		case 'q': return -1;
		}
	}
}
/*====================================
 * invoke_utils_menu -- Handle utilities menu
 *==================================*/
static void
invoke_utils_menu (void)
{
	int code;
	UIWINDOW uiwin=0;
	WINDOW *win=0;

	if (!utils_menu_win) {
		create_newwin2(&utils_menu_win, "utils_menu", 14, 66);
		utils_menu_win->outdated = true; /* needs drawing */
	}
	if (utils_menu_win->outdated) {
		repaint_utils_menu(utils_menu_win);
	}
	uiwin = utils_menu_win;
	win = uiw_win(uiwin);
	activate_uiwin(uiwin);

	wmove(win, 1, strlen(_(qSmn_uttl))+3);
	code = interact_choice_string(uiwin, "srRkidmeocq");
	deactivate_uiwin_and_touch_all();

	begin_action();
	switch (code) {
	case 's': save_gedcom(); break;
	case 'r': load_gedcom(false); break;
	case 'R': load_gedcom(true); break;
	case 'k': key_util(); break;
#if !defined(DEADENDS)
	case 'i': who_is_he_she(); break;
#endif
	case 'd': show_database_stats(); break;
#if !defined(DEADENDS)		/* DeadEnds has no cache */
	case 'm': display_cache_stats(); break;
	case 'e': edit_place_table(); break;
	case 'o': edit_user_options(); break;
#endif
	case 'c': invoke_cset_display(); break;
		/*
		we could add edit_global_config pretty easily, but the difficulty is
		that we don't know what to do about codeset with it :( [2002.06.18, Perry]
		*/
	case 'q': break;
	}
	end_action();
}
/*================================
 * invoke_extra_menu -- Handle extra menu
 *==============================*/
static int
invoke_extra_menu (RecordIndexEl **prec)
{
	int code;
	UIWINDOW uiwin=0;
	WINDOW *win=0;

	if (!extra_menu_win) {
		create_newwin2(&extra_menu_win, "extra_menu", 13,66);
		extra_menu_win->outdated = true; /* needs drawing */
	}
	if (extra_menu_win->outdated) {
		repaint_extra_menu(extra_menu_win);
	}
	uiwin = extra_menu_win;
	win = uiw_win(uiwin);

	while (1) {

		activate_uiwin(uiwin);
		wmove(win, 1, strlen(_(qSmn_xttl))+3);
		code = interact_choice_string(uiwin, "sex123456q");
		deactivate_uiwin_and_touch_all();

		switch (code) {
		case 's': return BROWSE_SOUR;
		case 'e': return BROWSE_EVEN;
		case 'x': return BROWSE_AUX;
		case '1': *prec = edit_add_source(); return BROWSE_SOUR;
		case '2': edit_source(NULL, false); return BROWSE_QUIT;
		case '3': *prec = edit_add_event(); return BROWSE_EVEN;
		case '4': edit_event(NULL, false); return BROWSE_QUIT;
		case '5': *prec = edit_add_other(); return BROWSE_AUX;
		case '6': edit_other(NULL, false); return BROWSE_QUIT;
		case 'q': return BROWSE_QUIT;
		}
	}
}
/*===============================
 * uopt_validate -- Validator when user edits 'user options table'
 *  returns descriptive string for failure, 0 for pass
 *=============================*/
static String
uopt_validate (TABLE tab, void * param)
{
	String codeset = valueof_str(tab, "codeset");
	String original_codeset = (String)param;
	/*
	our only rule currently is that user may not change codeset
	of a populated database
	*/
	if (!eqstr_ex(codeset, original_codeset)
		&& !eqstr_ex(codeset, int_codeset)) {
		if (num_indis()+num_fams()+num_sours()+num_evens()+num_othrs())
			return _("Impermissible to change codeset in a populated database");
	}
	return 0;
}

#if !defined(DEADENDS)
/*===============================
 * edit_place_table -- Allow user to edit the table of place abbreviations
 *=============================*/
static void
edit_place_table (void)
{
	edit_valtab_from_db("VPLAC", &placabbvs, ':', _(qSabverr), 0, 0);
}

/*===============================
 * edit_user_options -- Allow user to edit options embedded in current db
 *=============================*/
static void
edit_user_options (void)
{
	TABLE uopts = createStringTable();
	String param=0;
	get_db_options(uopts);
	param = valueof_str(uopts, "codeset");
	param = (param ? strsave(param) : 0);

	if (edit_valtab_from_db("VUOPT", &uopts, '=', _(qSuoperr), uopt_validate, (void *)param))
		set_db_options(uopts);
	strfree(&param);
	release_table(uopts);
}
#endif

/*============================================
 * get_answer -- Have user respond with string
 *  uiwin:   [IN] which window to use
 *  row:     [IN]  prompt location (vert)
 *  col:     [IN]  prompt location (horiz)
 *  buffer:  [OUT] response
 *  buflen:  [IN]  max size of response
 *  Has not been codeset-converted to internal yet
 *==========================================*/
bool
get_answer (UIWINDOW uiwin, int row, int col, String buffer, int buflen)
{
	WINDOW *win = uiw_win(uiwin);
	bool rtn = false;

	/* TODO: Is this necessary ? It prevents entering long paths */
	if (buflen > uiw_cols(uiwin)-col-1)
		buflen = uiw_cols(uiwin)-col-1;

	echo();
	wmove(win, row, col);
	if (wgetccnstr(win, buffer, buflen) != ERR)
		rtn = true;
	noecho();
	buffer[buflen-1] = 0; /* ensure zero-termination */

	return rtn;
}
/*================================================================
 * show_record -- Display record (any type) in requested mode
 *  uiwin:  [IN]  whither to draw
 *  key:    [IN]  key of record to display
 *  mode:   [IN]  what display mode (eg, vitals vs GEDCOM vs...)
 *  rect:   [IN]  where to draw
 *  scroll: [I/O] current scroll setting
 *  reuse:  [IN]  flag indicating if same record drawn last time
 * returns true if record was found, else FALSE (no record, nothing drawn)
 *==============================================================*/
bool
show_record (UIWINDOW uiwin, CString key, int mode, LLRECT rect
	, int * scroll, bool reuse)
{
	int row = rect->top;
	int hgt = rect->bottom - rect->top + 1;
	int width = rect->right - rect->left + 1;
	if (key[0]=='I') {
		RecordIndexEl *irec = keyToPersonRecord(key, currentDatabase);
		if (irec)
			show_indi(uiwin, irec, mode, rect, scroll, reuse);
		return irec != NULL;
	} else if (key[0]=='F') {
		RecordIndexEl *frec = keyToFamilyRecord(key, currentDatabase);
		if (frec)
			show_fam(uiwin, frec, mode, row, hgt, width, scroll, reuse);
		return frec != NULL;

	} else {
		/* could be S,E,X -- show_aux handles all of these */
		RecordIndexEl *rec = __llpy_key_to_record (key, NULL, currentDatabase);
		if (rec)
			show_aux(uiwin, rec, mode, rect, scroll, reuse);
		return rec != NULL;
	}
}
/*===================================================
 * message_string -- Return background message string
 *=================================================*/
String
message_string (void)
{
	if (!cur_screen) return "";
	if (cur_screen == MAIN_SCREEN)
		return _("DeadEnds -- Main Menu");
	ASSERT(cur_screen >= 1);
	ASSERT(cur_screen <= MAX_SCREEN);
	return get_screen_title(cur_screen);
}
/*=================================================
 * place_std_msg - Place standard message on screen
 *===============================================*/
void
place_std_msg (void)
{
	/* msg is placed on main window */
	UIWINDOW uiwin = main_win;
	WINDOW *win = uiw_win(uiwin);
	String str = message_string();
	int row = ll_lines-2;
	clear_hseg(win, row, 2, ll_cols-2);
	mvccwaddstr(win, row, 2, str);
	/* now we need to repaint main window, but if there are
	subwindows up, instead we call the touch_all routine,
	which does them all from ancestor to descendant */
	if (active_uiwin)
		touch_all(true);
	else
		wrefresh(win);
	place_cursor_main();
}
/*==================================+
 * rpt_print -- Implement report language print function
 * Created: 2003-02-01 (Perry Rapp)
 *=================================*/
void
rpt_print (CString str)
{
	llwprintf("%s", str);
}
/*=================================================
 * llvwprintf -- Called as wprintf(fmt, argp)
 *===============================================*/
void
llvwprintf (CString fmt, va_list args)
{
	UIWINDOW uiwin = stdout_win;
	if (uiwin) {
		WINDOW *win = uiw_win(uiwin);
		if (!stdout_vis) {
			clearw();
			activate_uiwin(uiwin);
		}
		vccwprintw(win, fmt, args);
		wrefresh(win);
	} else {
		vccprintf(fmt, args);
	}
	/*
	TO DO
	It would be nice to add this to the msg list
	but we need to deal with embedded carriage returns first
	so we can't do this yet. Also, if we do put it on the msg list,
	it is going to duplicate the stdout display currently being
	used (which is nicer looking, but scrolls off-screen).
	*/
/*	msg_outputv(MSG_ERROR, fmt, args);*/ /* also send to msg list */
}
/*==============================
 * clearw -- Clear stdout window
 *============================*/
void
clearw (void)
{
	UIWINDOW uiwin = stdout_win;
	WINDOW *win = uiw_win(uiwin);
	WINDOW *boxwin = uiw_boxwin(uiwin);
	uierase(uiwin);
	draw_win_box(boxwin);
	wmove(win, 0, 0);
	stdout_vis = true;
	wrefresh(boxwin);
}
/*=======================================
 * wfield -- Write field in stdout window
 *=====================================*/
void
wfield (int row, int col, String str)
{
	UIWINDOW uiwin = stdout_win;
	WINDOW *win = uiw_win(uiwin);
	if (!stdout_vis) clearw();
	mvccwaddstr(win, row, col, str);
	wrefresh(win);
}
/*===========================================
 * wpos -- Position to place in stdout window
 *=========================================*/
void
wpos (int row, int col)
{
	UIWINDOW uiwin = stdout_win;
	WINDOW *win = uiw_win(uiwin);
	wmove(win, row, col);
}
/*=======================================
 * show_horz_line -- Draw horizontal line
 *=====================================*/
void
show_horz_line (UIWINDOW uiwin, int row, int col, int len)
{
	WINDOW *win = uiw_win(uiwin);
	int i;
	mvwaddch(win, row, col, gr_ltee);
	for (i = 0; i < len-2; i++)
		waddch(win, gr_hline);
	waddch(win, gr_rtee);
}
/*=====================================
 * show_vert_line -- Draw vertical line
 *===================================*/
void
show_vert_line (UIWINDOW uiwin, int row, int col, int len)
{
	WINDOW *win = uiw_win(uiwin);
	int i;
	mvwaddch(win, row++, col, gr_ttee);
	for (i = 0; i < len-2; i++)
		mvwaddch(win, row++, col, gr_vline);
	mvwaddch(win, row, col, gr_btee);
}
/*=============================================
 * place_cursor_popup -- Move to cursor input location
 * For use with UIWINDOW menus -- popup menus which live
 * in their own private UIWINDOW (& curses) window,
 * such as the full scan popup
 *===========================================*/
void
place_cursor_popup (UIWINDOW uiwin)
{
	wmove(uiw_win(uiwin), uiw_cury(uiwin), uiw_curx(uiwin));
}
/*=============================================
 * place_cursor_main -- Move to idle cursor location
 * for use with main menu screens
 *===========================================*/
static void
place_cursor_main (void)
{
	int row=0, col = 30;
	DYNMENU dynmenu = get_screen_dynmenu(cur_screen);

	/* Hide/Display Cursor */
	if (dynmenu && dynmenu->hidden) {
		curs_set(0);
	} else {
		curs_set(1);
	}

	/* Position Cursor */
	switch (cur_screen) {
	case MAIN_SCREEN:   
		row = 5;
		col = strlen(_(qSplschs))+3;
		break;
	case LIST_SCREEN:
		listui_placecursor_main(&row, &col);
		break;
	case ONE_PER_SCREEN:
	case ONE_FAM_SCREEN:
	case AUX_SCREEN:
	case TWO_PER_SCREEN:
	case TWO_FAM_SCREEN:
		{
			/* These screens, which live on the main screen uiwindow,
			all use dynamic menus, and the cursor position in dynamic
			menus is controlled by the dynamic menu, because cursor
			moves up & down with dynamic menu */
			if (dynmenu->hidden) {
				/* no need to position cursor */
			} else {
				row = dynmenu->cur_y;
				col = dynmenu->cur_x;
			}
		}
		break;
	default:
		row = 1;
		col = 1;
		break;
	}
	wmove(uiw_win(main_win), row, col);
}
/*=============================================
 * dbprintf -- Debug printf(fmt, arg, arg, ...)
 *===========================================*/
void
dbprintf (String fmt, ...)
{
	va_list args;
	touchwin(uiw_win(debug_box_win));
	va_start(args, fmt);
	vccwprintw(uiw_win(debug_win), fmt, args);
	va_end(args);
	wrefresh(uiw_win(debug_box_win));
	sleep(2);
	touchwin(uiw_win(main_win));
	wrefresh(uiw_win(main_win));
}
/*==================================================
 * do_edit -- Shift to user's screen editor and back
 *================================================*/
void
do_edit (void)
{
	int rtn=-1;

	endwin();
#ifdef WIN32
	/* use w32system, because it will wait for the editor to finish */
	rtn = w32system(editstr);
#else
	rtn = system(editstr);
#endif
	if (rtn != 0) {
		printf(_("Editor or system call failed."));
		puts("");
		sleep(2);
	}

	clearok(curscr, 1);
	place_cursor_main();
	wrefresh(curscr);
	noecho();
}
/*================================================
 * mvwaddstr_lim -- output a string, like mvwaddstr
 *  except trim it to no more than maxlen wide
 *==============================================*/
static void
mvwaddstr_lim (WINDOW *wp, int x, int y, char *cp, int maxlen)
{
	char buffer[60];
	if ((int)strlen(cp)<=maxlen)
		mvccwaddstr(wp, x, y, cp);
	else {
		if (maxlen > (int)sizeof(buffer)-1)
			maxlen = sizeof(buffer)-1;
		destrncpy(buffer, cp, maxlen-1, uu8);
		strcat(buffer, "*");
		mvccwaddstr(wp, x, y, buffer);
	}
}
/*================================================
 * check_menu -- update menu layout info
 *  (in case just resized)
 * Created: 2002/10/24 (Perry Rapp)
 *==============================================*/
static void
check_menu (DYNMENU dynmenu)
{
	/* (reserve spots for ? and q on each page) */
	dynmenu->pageitems = dynmenu->rows*dynmenu->cols-2;
	dynmenu->pages = (dynmenu->size-1)/dynmenu->pageitems+1;
	if (dynmenu->size < dynmenu->pageitems + 1) { /* don't need ? if they fit */
		dynmenu->pages = 1;
		dynmenu->page = 0;
	}
}
/*================================================
 * output_menu -- print menu array to screen
 * Caller specifies bottom row to use, & width
 * Menu structure contains all menu items & # of
 * columns to use
 *==============================================*/
static void
output_menu (UIWINDOW uiwin, DYNMENU dynmenu)
{
	WINDOW *win = uiw_win(uiwin);
	int row;
	int icol=0;
	int col=3;
	/* more legible names */
	int MenuSize = dynmenu->size;
	int MenuCols = dynmenu->cols;
	int MenuPage = dynmenu->page;
	int MenuPages = dynmenu->pages;
	int pageitems = dynmenu->pageitems;
	MENUSET menuset = dynmenu_get_menuset(dynmenu);
	MenuItem ** items = menuset_get_items(menuset);
	int width = dynmenu->width;
	/* reserve 2 spaces at each end, and one space in front of each Col */
	int colwidth = (width-4)/MenuCols-1;
	int Item = 0;
	Item = MenuPage * pageitems;
	if (Item >= MenuSize)
		Item = ((MenuSize-1)/pageitems)*pageitems;
	icol = 0;
	col = 3;
	row = dynmenu->top;
	/* now display all the menu items we can fit on this page */
	while (1)
	{
		mvwaddstr_lim(win, row, col, items[Item++]->LocalizedDisplay, colwidth);
		if (Item == MenuSize)
			break;
		row++;
		if (icol<MenuCols-1 && row>dynmenu->bottom)
		{
			icol++;
			col += colwidth+1;
			row = dynmenu->top;
			continue;
		}
		if (MenuPages == 1) {
			/* one slot reserved for "q" */
			if (icol==MenuCols-1 && row==dynmenu->bottom)
				break;
		} else {
			/* two slots reserved, "q" & "?" */
			if (icol==MenuCols-1 && row==dynmenu->bottom-1)
				break;
		}
	}
	/* print the "q" and "?" items */
	row = dynmenu->bottom-1;
	col = 3+(MenuCols-1)*(colwidth+1);
	if (dynmenu->pages > 1)
		mvwaddstr_lim(win, row, col, g_MenuItemOther.LocalizedDisplay, colwidth);
	mvwaddstr_lim(win, ++row, col, g_MenuItemQuit.LocalizedDisplay, colwidth);
}
/*==================================================================
 * toggle_browse_menu - toggle display of menu at bottom of screen
 *================================================================*/
void
toggle_browse_menu (void)
{
	dynmenu_toggle_menu(get_screen_dynmenu(cur_screen));
}
/*==================================================================
 * cycle_browse_menu() - show other menu choices on browse menu
 *================================================================*/
void
cycle_browse_menu (void)
{
	dynmenu_next_page(get_screen_dynmenu(cur_screen));
}
/*==================================================================
 * adjust_browse_menu_height - Change height of current browse screen menu
 *================================================================*/
void
adjust_browse_menu_height (int delta)
{
	dynmenu_adjust_height(get_screen_dynmenu(cur_screen), delta);
}
/*==================================================================
 * adjust_browse_menu_cols - Change # of columns in current menu
 *================================================================*/
void
adjust_browse_menu_cols (int delta)
{
	dynmenu_adjust_menu_cols(get_screen_dynmenu(cur_screen), delta);
}
/*=========================================
 * get_brwsmenu_size -- How many lines does browse menu take ?
 *=======================================*/
static int
get_brwsmenu_size (int screen)
{
	DYNMENU dynmenu = get_screen_dynmenu(screen);
	return dynmenu->hidden ? EMPTY_MENU : dynmenu->rows+1;
}
/*=====================
 * clear_stdout_hseg -- clear a horizontal line segment on stdout win
 *====================*/
void
clear_stdout_hseg (int row, int x1, int x2)
{
	UIWINDOW uiwin = stdout_win;
	WINDOW *win = uiw_win(uiwin);
	clear_hseg(win, row, x1, x2);
}
/*=====================
 * clear_hseg -- clear a horizontal line segment
 *  (used for partial screen clears)
 *====================*/
void
clear_hseg (WINDOW *win, int row, int x1, int x2)
{
	/* workaround for curses bug with spacs */
	if (getdeoptint("ForceScreenErase", 0) > 0) {
		/* fill virtual output with dots */
		color_hseg(win, row, x1, x2, '_');
		wnoutrefresh(win);
		/* now fill it back with spaces */
		color_hseg(win, row, x1, x2, ' ');
		wrefresh(win);
	} else {
		color_hseg(win, row, x1, x2, ' ');
	}
}
/*=====================
 * color_hseg -- fill a horizontal line segment
 *  (used for clearing)
 *====================*/
static void
color_hseg (WINDOW *win, int row, int x1, int x2, char ch)
{
	int i;
	for (i=x1; i<=x2; ++i)
		mvwaddch(win, row, i, ch);
}

/* display_status -- put string in status line
   We don't touch the status_transitory flag
   That is caller's responsibility. */

static void
display_status (String text)
{
  UIWINDOW uiwin = main_win;
  WINDOW *win = uiw_win(uiwin);
  int row;
  /* first store it */
  destrncpy(status_showing, text, sizeof(status_showing), uu8);
  if ((int)strlen(text)>ll_cols-6) {
    status_showing[ll_cols-8] = 0;
    strcat(status_showing, "...");
  }
  /* then display it */
  row = ll_lines-2;
  clear_hseg(win, row, 2, ll_cols-2);
  wmove(win, row, 2);
  mvccwaddstr(win, row, 2, status_showing);
  place_cursor_main();
  wrefresh(win);
}

/*=====================================
 * msg_width -- get max width of msgs
 *===================================*/
int
msg_width (void)
{
	return ll_cols-6;
}
#if 1			   /* for now... */
/*=========================================
 * curses_outputv -- output message varargs style arguments
 * Actually all other msg functions delegate to here.
 *  @level:     -1=error,0=info,1=status
 *  @fmt:   [IN]  printf style format string
 *  @args:  [IN]  vprintf style varargs
 * Puts into message list and/or into status area
 *=======================================*/
void
curses_outputv (ARG_UNUSED(void *data),
		MSG_LEVEL level, CString fmt, va_list args)
{
	char buffer[250];
	String ptr;
	unsigned int width = MAINWIN_WIDTH-5;
	/* prefix errors & infos with * and space respectively */
	switch(level) {
		case MSG_ERROR:
			buffer[0] = '*';
			ptr = &buffer[1];
			break;
		case MSG_INFO:
			buffer[0] = ' ';
			ptr = &buffer[1];
			break;
		default:
			ptr = buffer;
			break;
	}
	/* now make string to show/put on msg list */
	destrncpyvf(ptr, sizeof(buffer)-1, uu8, fmt, args);
	/* first handle transitory/status messages */
	if (level==MSG_STATUS) {
		if (lock_std_msg)
			return; /* can't display it, status bar is locked */
		if (status_showing[0] && !status_transitory) {
			/* we are overwriting something important
			so it is already on the msg list, we just need to make
			sure the msg list gets displayed */
			if (!viewing_msgs)
				msg_flag = true;
		}
		display_status(buffer);
		return;
	}
	/* everything important goes onto msg list */
	append_to_msg_list(buffer);
	/* update flag about whether we need to show msg list to user */
	/* being careful in case we are currently *in* the msg list
	show routine */
	if (!viewing_msgs && (lengthList(msg_list)>1 || lock_std_msg)) {
		msg_flag = true;
	}
	/* now put it to status area if appropriate */
	if (!lock_std_msg) {
		if (strlen(buffer)>width) {
			buffer[width-4]=0;
			strcat(buffer, "...");
/*
TODO: This doesn't make sense until the msg list handles long strings
			if (!viewing_msgs)
				msg_flag = true;
*/
		}
		display_status(buffer);
	}
}
/*=========================================
 * msg_impl -- put msg on the msg list
 * This is a list that we show the user
 * when the current command completes,
 * unless it only had one item, and it got
 * put on the status bar, and it wasn't too wide.
 *=======================================*/
static void
append_to_msg_list (String msg)
{
		if (!msg_list)
			msg_list = createList (NULL, NULL, free, false);
		enqueue_list(msg_list, strsave(msg));
}
#endif
/*=========================================
 * begin_action -- prepare to process users choice
 *=======================================*/
static
void begin_action (void)
{
	clear_msgs();
}
/*=========================================
 * end_action -- finished processing users choice
 *  show msg list if appropriate
 *=======================================*/
static
void end_action (void)
{
	/* pause for keypress for finish stdout/err if appropriate */
	check_stdout();
	/* put up list of errors if appropriate */
	if (msg_flag && msg_list) {
		String * strngs = (String *)stdalloc(lengthList(msg_list)*sizeof(String));
		int i=0;
		FORLIST(msg_list, el)
			strngs[i++] = el;
		ENDLIST
		viewing_msgs = true; /* suppress msg generation */
		view_array(_(qSerrlist), lengthList(msg_list), strngs);
		viewing_msgs = false;
		stdfree(strngs);
		clear_msgs();
	}
}
/*=========================================
 * clear_msgs -- delete msg list
 *  The msg list holds messages when several
 *  occurred during the last operation
 *=======================================*/
static void
clear_msgs (void)
{
	if (msg_list) {
		destroy_list(msg_list);
		msg_list = 0;
	}
	msg_flag = false;
	/* also clear status bar */
	clear_status();
}
/*=========================================
 * clear_status -- clear status string
 *  The status string is the last message displayed
 *  and is shown at the very bottom of the main screen
 *=======================================*/
static void
clear_status (void)
{
	if (!lock_std_msg)
		status_showing[0]=0;
}
/*=========================================
 * lock_status_msg -- temporarily hold status message
 *=======================================*/
void
lock_status_msg (bool lock)
{
	lock_std_msg = lock;
}
/*=====================================
 * repaint_add_menu -- Draw menu choices for main add item menu
 *===================================*/
static void
repaint_add_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	int row = 1;
	uierase(uiwin);
	draw_win_box(win);
	mvccwaddstr(win, row++, 2, _(qSmn_add_ttl));
	mvccwaddstr(win, row++, 4, _(qSmn_add_indi));
	mvccwaddstr(win, row++, 4, _(qSmn_add_fam));
	mvccwaddstr(win, row++, 4, _(qSmn_add_chil));
	mvccwaddstr(win, row++, 4, _(qSmn_add_spou));
	mvccwaddstr(win, row++, 4, _(qSmn_ret));
	uiwin->outdated = false;
}
/*=====================================
 * repaint_delete_menu -- Draw menu choices for main delete item menu
 *===================================*/
static void
repaint_delete_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	int row = 1;
	uierase(uiwin);
	draw_win_box(win);
	mvccwaddstr(win, row++, 2, _(qSmn_del_ttl));
	mvccwaddstr(win, row++, 4, _(qSmn_del_chil));
	mvccwaddstr(win, row++, 4, _(qSmn_del_spou));
	mvccwaddstr(win, row++, 4, _(qSmn_del_indi));
	mvccwaddstr(win, row++, 4, _(qSmn_del_fam));
	mvccwaddstr(win, row++, 4, _(qSmn_del_any));
	mvccwaddstr(win, row++, 4, _(qSmn_ret));
	uiwin->outdated = false;
}
/*=====================================
 * repaint_utils_menu --
 *===================================*/
static void
repaint_utils_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	int row = 1;
	uierase(uiwin);
	draw_win_box(win);
	mvccwaddstr(win, row++, 2, _(qSmn_uttl));
	mvccwaddstr(win, row++, 4, _(qSmn_utsave));
	mvccwaddstr(win, row++, 4, _(qSmn_utread));
	mvccwaddstr(win, row++, 4, _(qSmn_utgdchoo));
	mvccwaddstr(win, row++, 4, _(qSmn_utkey));
#if !defined(DEADENDS)
	mvccwaddstr(win, row++, 4, _(qSmn_utkpers));
#endif
	mvccwaddstr(win, row++, 4, _(qSmn_utdbstat));
	mvccwaddstr(win, row++, 4, _(qSmn_utmemsta));
#if !defined(DEADENDS)
	mvccwaddstr(win, row++, 4, _(qSmn_utplaces));
#endif
	mvccwaddstr(win, row++, 4, _(qSmn_utusropt));
	mvccwaddstr(win, row++, 4, _(qSmn_mmcset));
	mvccwaddstr(win, row++, 4, _(qSmn_quit));
	uiwin->outdated = false;
}
/*=====================================
 * repaint_extra_menu --
 *===================================*/
static void
repaint_extra_menu (UIWINDOW uiwin)
{
	WINDOW *win = uiw_win(uiwin);
	int row = 1;
	uierase(uiwin);
	draw_win_box(win);
	mvccwaddstr(win, row++, 2, _(qSmn_xttl));
	mvccwaddstr(win, row++, 4, _(qSmn_xxbsour));
	mvccwaddstr(win, row++, 4, _(qSmn_xxbeven));
	mvccwaddstr(win, row++, 4, _(qSmn_xxbothr));
	mvccwaddstr(win, row++, 4, _(qSmn_xxasour));
	mvccwaddstr(win, row++, 4, _(qSmn_xxesour));
	mvccwaddstr(win, row++, 4, _(qSmn_xxaeven));
	mvccwaddstr(win, row++, 4, _(qSmn_xxeeven));
	mvccwaddstr(win, row++, 4, _(qSmn_xxaothr));
	mvccwaddstr(win, row++, 4, _(qSmn_xxeothr));
	mvccwaddstr(win, row++, 4, _(qSmn_quit));
	uiwin->outdated = false;
}
/*============================
 * activate_uiwin --
 *  push new uiwindow on top of current one
 *==========================*/
void
activate_uiwin (UIWINDOW uiwin)
{
	WINDOW * win = uiw_win(uiwin);
	ASSERT(uiwin);
	ASSERT(win);
	ASSERT(!uiw_parent(uiwin));

	/* link into parent/child chain */
	uiw_parent(uiwin) = active_uiwin;
	if (active_uiwin) {
		ASSERT(!uiw_child(active_uiwin));
		uiw_child(active_uiwin) = uiwin;
		/* refresh current (in case it was obscured by stdout */
		wrefresh(uiw_win(active_uiwin));
	}

	/* switch to new & refresh */
	active_uiwin = uiwin;
	touchwin(win);
	wrefresh(win);
}
/*============================
 * reactivate_uiwin --
 *  push new window on top, if not already on top
 *  and refresh it in any case
 *==========================*/
#ifdef UNUSED_CODE
static void
reactivate_uiwin (UIWINDOW uiwin)
{
	if (active_uiwin != uiwin)
		activate_uiwin(uiwin);
	else {
		WINDOW * win = uiw_win(uiwin);
		touchwin(win);
		wrefresh(win);
	}
}
#endif
/*============================
 * deactivate_uiwin -- Remove currently active
 *  and pop to its parent (if it has one)
 *==========================*/
static void
deactivate_uiwin (void)
{
	UIWINDOW uiw = active_uiwin;
	active_uiwin = uiw_parent(active_uiwin);
	if (active_uiwin) {
		ASSERT(uiw_child(active_uiwin)==uiw);
	}
	uiw_parent(uiw)=0;
	if (uiw_dynamic(uiw))
		delete_uiwindow(&uiw);
	if (active_uiwin && uiw_child(active_uiwin)) {
		uiw_child(active_uiwin)=0;
	}
}
/*=============================================
 * deactivate_uiwin_and_touch_all --
 *  remove current window & repaint ones left
 *===========================================*/
void
deactivate_uiwin_and_touch_all (void)
{
	deactivate_uiwin();
	if (active_uiwin)
		touch_all(true);
}
/*============================
 * touch_all -- Repaint all ancestors of current window
 * from furthest to nearest
 *==========================*/
static void
touch_all (bool includeCurrent)
{
	UIWINDOW uiwin=active_uiwin;
	ASSERT(uiwin);
	/* climb to highest window ancestor */
	while (uiw_parent(uiwin)) {
		uiwin = uiw_parent(uiwin);
	}
	/* walk down touching */
	while (uiwin && (includeCurrent || uiwin!=active_uiwin)) {
		touchwin(uiw_win(uiwin));
		wrefresh(uiw_win(uiwin));
		uiwin = uiw_child(uiwin);
	}
}
/*============================
 * switch_to_uiwin --
 *  switch away from currently active uiwin
 *  to new uiwin
 *  currently active uiwin (if any) must be solo
 *  new uiwin must be solo
 *==========================*/
static void
switch_to_uiwin (UIWINDOW uiwin)
{
	WINDOW * win = uiw_win(uiwin);
	if (uiwin != active_uiwin)
	{
		ASSERT(uiwin);
		ASSERT(win);	
		ASSERT(!uiw_parent(uiwin));
		ASSERT(!uiw_child(uiwin));

		/* link into parent/child chain */
		uiw_parent(uiwin) = active_uiwin;
		if (active_uiwin)
		{
			/* current active window must be solo, no parent or child */
			ASSERT(!uiw_child(active_uiwin));
			ASSERT(!uiw_parent(active_uiwin));
		}
		/* switch to new & refresh */
		active_uiwin = uiwin;
	}
	touchwin(win);
	wrefresh(win);
}
/*============================
 * refresh_stdout --
 *  bring stdout to front
 *==========================*/
void
refresh_stdout (void)
{
	wrefresh(uiw_win(stdout_win));
}
/*============================
 * call_system_cmd --
 *  execute a shell command (for report interpreter)
 *==========================*/
void
call_system_cmd (CString cmd)
{
	int rtn=-1;

	endwin();
#ifndef WIN32
	rtn = system("clear");
#endif
	rtn = system(cmd);

	if (rtn != 0) {
		printf(_("System command failed."));
		puts("");
		sleep(2);
	}
	clearok(curscr, 1);
	place_cursor_main();
	wrefresh(curscr);
}
/*============================
 * uierase -- erase window
 *  handles manual erasing if broken_curses flag set
 *==========================*/
void
uierase (UIWINDOW uiwin)
{
	LLRECT rect = 0;
	wipe_window_rect(uiwin, rect);
}
/*================================================
 * wipe_window_rect -- Clear a rectangle in a window
 *  handle curses space bug
 *==============================================*/
void
wipe_window_rect (UIWINDOW uiwin, LLRECT rect)
{
	WINDOW * win = uiw_win(uiwin);
	/* workaround for curses bug with spaces */
	if (getdeoptint("ForceScreenErase", 0) > 0) {
		/*
		To fix the dirty output on a redhat 6 system
		(with ncurses-5.2-8), required the call to
		redrawwin, instead of using wrefresh.
		Perry, 2002.05.27
		*/
		/*
		uicolor(uiwin, rect, '=');
		wnoutrefresh(win);
		*/
		/* now fill it back with spaces */
		uicolor(uiwin, rect, ' ');
		redrawwin(win);
	} else {
		/* fill it back with spaces */
		if (rect)
			uicolor(uiwin, rect, ' ');
		else
			werase(win); /* let curses do it */
	}
}
/*============================
 * uicolor -- fill window with character
 *  if rect is nonzero, fill that rectangular area
 *  if rect is zero, fill entire window
 *==========================*/
static void
uicolor (UIWINDOW uiwin, LLRECT rect, char ch)
{
	int i;
	WINDOW *win = uiw_win(uiwin);
	struct tag_llrect rects;

	if (!rect) {
		rects.top = 0;
		rects.bottom = uiw_rows(uiwin)-1;
		rects.left = 0;
		rects.right = uiw_cols(uiwin)-1;
		rect = &rects;
	}
	for (i=rect->top; i <= rect->bottom; ++i) {
		color_hseg(win, i, rect->left, rect->right, ch);
	}
}
/*==================================================
 * platform_postcurses_init -- platform-specific code
 *  coming after curses initialized
 *================================================*/
static void
platform_postcurses_init (void)
{
#ifdef WIN32
	char buffer[80];
	String title = _(qSmtitle);
	snprintf(buffer, sizeof(buffer), title, get_deadends_version(sizeof(buffer)-1-strlen(title)));
	wtitle(buffer);
#endif
}
/*==================================================
 * get_main_screen_width -- current width of main screen
 *================================================*/
int
get_main_screen_width (void)
{
	return MAINWIN_WIDTH;
}
/*==================================================
 * get_gr_ttee -- current character used for box corners
 *================================================*/
llchtype
get_gr_ttee (void)
{
	return gr_ttee; /* eg, '+' */
}
/*==================================================
 * clear_status_display -- clear any lingering status display
 * (called by interact code after user has pressed a button)
 *================================================*/
void
clear_status_display (void)
{
	if (programRunning) return;
	if (lock_std_msg) return;
	if (status_showing[0]) {
		status_showing[0] = 0;
		place_std_msg();
	}
}
/*============================
 * register_screen_lang_callbacks -- (un)register our callbacks
 *  for language or codeset changes
 *==========================*/
static void
register_screen_lang_callbacks (bool registering)
{
	if (registering) {
		register_uilang_callback(screen_on_lang_change, 0);
		register_uicodeset_callback(screen_on_lang_change, 0);
	} else {
		unregister_uilang_callback(screen_on_lang_change, 0);
		unregister_uicodeset_callback(screen_on_lang_change, 0);
	}
}
/*============================
 * screen_on_lang_change -- UI language or codeset has changed
 *==========================*/
static void
screen_on_lang_change (ATTRIBUTE_UNUSED void *uparm)
{
#if defined(DEADENDS)
  int max = lengthList (list_uiwin);
  int ndx;

  for (ndx = 0; ndx < max; ndx++)
    {
      UIWINDOW uiwin = (UIWINDOW) getListElement (list_uiwin, ndx);
      uiwin->outdated = true;
    }
#else
	LIST_ITER listit=0;
	void *ptr=0;
	listit = begin_list(list_uiwin);
	while (next_list_ptr(listit, &ptr)) {
		UIWINDOW uiwin = (UIWINDOW)ptr;
		uiwin->outdated = true;
	}
	end_list_iter(&listit);
#endif
}

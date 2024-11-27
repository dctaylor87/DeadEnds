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
/*==============================================================
 * screen.h -- Header file for curses-based screen I/O
 * Copyright (c) 1991-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 20 Aug 93
 *   3.0.0 - 26 Jul 94    3.0.2 - 16 Dec 94
 *============================================================*/

#ifndef _SCREEN_H
#define _SCREEN_H

#include "mycurses.h"

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif

//#ifndef TRUE
//#       define TRUE ((BOOLEAN)1)
//#endif
//#ifndef FALSE
//#       define FALSE ((BOOLEAN)0)
//#endif


/* Support for 'broken' curses implementations */
/* That don't define ACS_xxx constants */
#ifndef ACS_TTEE
#   undef ACS_TTEE
#	define ACS_TTEE '+'
#   undef ACS_RTEE
#	define ACS_RTEE '+'
#   undef ACS_LTEE
#	define ACS_LTEE '+'
#   undef ACS_BTEE
#	define ACS_BTEE '+'
#   undef ACS_VLINE 
#	define ACS_VLINE '|'
#   undef ACS_HLINE
#	define ACS_HLINE '-'
#   undef ACS_LLCORNER
#   define ACS_LLCORNER '*'
#   undef ACS_LRCORNER
#   define ACS_LRCORNER '*'
#   undef ACS_ULCORNER
#   define ACS_ULCORNER '*'
#   undef ACS_URCORNER
#   define ACS_URCORNER '*'
#endif

/* box drawing character type */
typedef unsigned long llchtype;

/*=========================================
 * UIWINDOWs -- Main screen, menus and popups
 *=======================================*/
/* wrapper for WINDOW */
typedef struct tag_uiwindow {
	CString name;       /* non-heap name (for debugging) */
	WINDOW * win;      /* curses window */
	WINDOW * boxwin;   /* surrounding window just for boxing */
	struct tag_uiwindow * parent; /* fixed or dynamic parent */
	struct tag_uiwindow * child;
	bool permsub;   /* TRUE if a fixed subwindow */
	bool dynamic;   /* TRUE means delete when finished */
	bool outdated;  /* for language changes */
	int rows;
	int cols;
	int cur_y;         /* row for input cursor */
	int cur_x;         /* col for input cursor */
} * UIWINDOW;
#define uiw_win(x)      ((x)->win)
#define uiw_boxwin(x)   ((x)->boxwin)
#define uiw_parent(x)   ((x)->parent)
#define uiw_child(x)    ((x)->child)
#define uiw_permsub(x)  ((x)->permsub)
#define uiw_dynamic(x)  ((x)->dynamic)
#define uiw_rows(x)     ((x)->rows)
#define uiw_cols(x)     ((x)->cols)
#define uiw_cury(x)     ((x)->cur_y)
#define uiw_curx(x)     ((x)->cur_x)

extern int ll_lines; /* number of lines used by LifeLines (usually LINES) */
extern int ll_cols;  /* number of columns used by LifeLines (usually COLSREQ) */
extern int cur_screen;
extern UIWINDOW stdout_win;
extern UIWINDOW main_win;

/* Keep this in sync with llinesi.h */
#ifndef llinesi_h_included
typedef struct tag_llrect {
        int top;
        int bottom;
        int left;
        int right;
} *LLRECT;
#endif

enum {
        BROWSE_INDI
        , BROWSE_FAM
        , BROWSE_PED
        , BROWSE_TAND
        , BROWSE_QUIT
        , BROWSE_2FAM
        , BROWSE_LIST
        , BROWSE_AUX
        , BROWSE_EVEN
        , BROWSE_SOUR
        , BROWSE_UNK
};

struct tag_menuset;

/*
  Function Prototypes, alphabetical by module
*/

/* loadsave.c */
void load_gedcom(bool picklist);
bool save_gedcom(void);

/* screen.c */
void activate_uiwin(UIWINDOW uiwin);
void adjust_browse_menu_cols(int delta);
void adjust_browse_menu_height(int delta);
int aux_browse(RecordIndexEl *rec, int mode, bool reuse);
void clear_hseg(WINDOW *, int row, int x1, int x2);
void clear_stdout_hseg(int row, int x1, int x2);
void create_newwin2(UIWINDOW * puiw, CString name, int rows, int cols);
void cycle_browse_menu(void);
void deactivate_uiwin_and_touch_all(void);
void display_2fam(RecordIndexEl *frec1, RecordIndexEl *frec2, int mode);
void display_2indi(RecordIndexEl *irec1, RecordIndexEl *irec2, int mode);
void display_fam(RecordIndexEl *fam, int mode, bool reuse);
void display_indi(RecordIndexEl *indi, int mode, bool reuse);
void display_screen(int);
void dbprintf(String, ...) ATTRIBUTE_PRINTF(1,2);
void draw_win_box(WINDOW * win);
llchtype get_gr_ttee(void);
int get_main_screen_width(void);
int get_uitime(void);
int init_screen(char * errmsg, int errsize);
int interact_2fam(void);
int interact_2indi(void);
int interact_fam(void);
int interact_indi(void);
int interact_popup(UIWINDOW uiwin, String str);
int list_browse(Sequence *seq, int top, int *cur, int mark);
void lock_status_msg(bool lock);
void main_menu(void);
String message_string (void);
void paint_main_screen(void);
void paint_two_fam_screen(void);
void set_screen_graphical(bool graphical);
void show_horz_line(UIWINDOW, int, int, int);
void show_indi(UIWINDOW uiwin, RecordIndexEl *indi, int mode, LLRECT
	, int * scroll, bool reuse);
void show_indi_vitals(UIWINDOW uiwin, RecordIndexEl *irec, LLRECT,
		      int *scroll, bool reuse);
bool show_record(UIWINDOW uiwin, CString key, int mode, LLRECT
	, int * scroll, bool reuse);
void show_vert_line(UIWINDOW, int, int, int);
void term_screen(void);
void toggle_browse_menu(void);
int twofam_browse(GNode *, GNode *, int mode);
int twoindi_browse(GNode *, GNode *, int mode);
void uierase(UIWINDOW uiwin);
void wfield(int, int, CString);
void wipe_window_rect(UIWINDOW uiwin, LLRECT rect);
void wpos (int, int);


/* show.c (curses specific) */
extern int Scroll1;
void init_show_module(void);
void show_ancestors (UIWINDOW uiwin, RecordIndexEl *irec, LLRECT
	, int * scroll, bool reuse);
void show_aux(UIWINDOW uiwin, RecordIndexEl *rec, int mode, LLRECT
	, int * scroll, bool reuse);
void show_big_list(Sequence *, int, int, int);
void show_childnumbers(void);
void show_descendants(UIWINDOW uiwin, RecordIndexEl *rec, LLRECT
	, int * scroll, bool reuse);
void show_fam_vitals (UIWINDOW uiwin, RecordIndexEl *frec, int row, int hgt
	, int width, int *scroll, bool reuse);
void show_gedcom (UIWINDOW uiwin, RecordIndexEl *rec, int gdvw, LLRECT
	, int * scroll, bool reuse);
void show_reset_scroll(void);
void show_sour_display(GNode *, int, int);
void show_scroll(int delta);
void show_scroll2(int delta);
void switch_scrolls(void);
void term_show_module(void);

#ifndef _FEEDBACK_H
#include "feedback.h"
#endif /* _FEEDBACK_H */

#endif /* _SCREEN_H */

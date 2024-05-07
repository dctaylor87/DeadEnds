/*
  Functions used inside the liflines module
  The high-level but not curses-specific portion of the program
  screen.h is not included before this header, so UIWIN etc cannot
  be used here (see screeni.h for declarations requiring screen.h).
*/

#ifndef llinesi_h_included
#define llinesi_h_included

#if !defined(DEADENDS)
#ifndef _GEDCOM_H
#include "gedcom.h"
#endif

#ifndef _INDISEQ_H
#include "indiseq.h"
#endif
#endif

typedef struct tag_llrect {
	INT top;
	INT bottom;
	INT left;
	INT right;
} *LLRECT;

struct tag_import_feedback;
#ifndef IMPORT_FEEDBACK_type_defined
typedef struct tag_import_feedback *IMPORT_FEEDBACK;
#define IMPORT_FEEDBACK_type_defined
#endif

struct tag_export_feedback;

/* add.c */
#if defined(DEADENDS)
RECORD add_family_by_edit(RECORD sprec1, RECORD sprec2, RECORD chrec, bool rfmt);
RECORD add_indi_by_edit(bool rfmt);
#else
RECORD add_family_by_edit(RECORD sprec1, RECORD sprec2, RECORD chrec, RFMT rfmt);
RECORD add_indi_by_edit(RFMT rfmt);
#endif
BOOLEAN add_indi_no_cache(NODE);
CString get_unresolved_ref_error_string(INT count);
#if defined(DEADENDS)
NODE prompt_add_child(NODE child, NODE fam, bool rfmt);
#else
NODE prompt_add_child(NODE child, NODE fam, RFMT rfmt);
#endif
BOOLEAN prompt_add_spouse(RECORD spouse, RECORD fam, BOOLEAN conf);

/* advedit.c */
void advanced_person_edit(NODE);
void advanced_family_edit(NODE);

/* ask.c */
RECORD ask_for_any(CString ttl, ASK1Q ask1);
INDISEQ ask_for_indiseq(CNSTRING ttl, char ctype, INT *prc);

/* browse.c */
RECORD choose_any_event(void);
RECORD choose_any_other(void);
RECORD choose_any_source(void);
INT get_chist_len(void);
INDISEQ get_chistory_list(void);
INT get_vhist_len(void);
INDISEQ get_vhistory_list(void);
BOOLEAN handle_fam_mode_cmds(INT c, INT * mode);
BOOLEAN handle_indi_mode_cmds(INT c, INT * mode);
BOOLEAN handle_menu_cmds(INT c, BOOLEAN * reuse);
BOOLEAN handle_scroll_cmds(INT c, BOOLEAN * reuse);
void history_record_change(RECORD);
void init_browse_module(void);
void main_browse(RECORD, INT code);
NODE my_prompt_add_child(NODE child, NODE fam);
void term_browse_module(void);

/* delete.c */
void choose_and_remove_indi(NODE indi, CONFIRMQ confirmq);
void choose_and_delete_family(void);
BOOLEAN choose_and_remove_child(RECORD irec, RECORD frec, BOOLEAN nolast);
BOOLEAN choose_and_remove_spouse(RECORD irec, RECORD frec, BOOLEAN nolast);
BOOLEAN choose_and_remove_any_record(RECORD rec, CONFIRMQ confirmq);

/* edit.c */
#if defined(DEADENDS)
BOOLEAN edit_family(RECORD frec1, bool rfmt);
BOOLEAN edit_indi(RECORD irec1, bool rfmt);
#else
BOOLEAN edit_family(RECORD frec1, RFMT rfmt);
BOOLEAN edit_indi(RECORD irec1, RFMT rfmt);
#endif

/* lbrowse.c */
INT browse_list(RECORD *prec1, RECORD *prec2, INDISEQ *pseq);

/* lines_usage.c */
void print_lines_usage(CNSTRING exename);


/* merge.c */
RECORD merge_two_indis(NODE, NODE, BOOLEAN);
RECORD merge_two_fams(NODE, NODE);

/* miscutls.c */
void key_util(void);
void show_database_stats(void);
#if !defined(DEADENDS)
void who_is_he_she(void);
#endif
void sighand_cursesui(int sig);
void sighand_cmdline(int sig);

/* newrecs.c */
RECORD edit_add_event(void);
RECORD edit_add_other(void);
RECORD edit_add_source(void);
#if defined(DEADENDS)
BOOLEAN edit_any_record(RECORD rec, bool rfmt);
BOOLEAN edit_event(RECORD rec, bool rfmt);
BOOLEAN edit_other(RECORD rec, bool rfmt);
BOOLEAN edit_source(RECORD rec, bool rfmt);
#else
BOOLEAN edit_any_record(RECORD rec, RFMT rfmt);
BOOLEAN edit_event(RECORD rec, RFMT rfmt);
BOOLEAN edit_other(RECORD rec, RFMT rfmt);
BOOLEAN edit_source(RECORD rec, RFMT rfmt);
#endif

/* pedigree.c */
	/* gedcom view mode */
enum { GDVW_NORMAL, GDVW_EXPANDED, GDVW_TEXT };
	/* data for output canvas */
	/* NB: pedigree will adjust scroll if out of limits */
	struct tag_canvasdata;
		/* callback to output a line */
	typedef void (*PEDLINE)(struct tag_canvasdata * canvas, INT x, INT y
		, STRING string, INT overflow);
		/* collection of data needed by pedigree */
	typedef struct tag_canvasdata { LLRECT rect; INT scroll; void * param;
		PEDLINE line; } *CANVASDATA;
	/* functions */
void pedigree_draw_ancestors(RECORD rec, CANVASDATA canvasdata, BOOLEAN reuse);
void pedigree_draw_descendants(RECORD rec, CANVASDATA canvasdata, BOOLEAN reuse);
void pedigree_draw_gedcom(RECORD rec, INT gdvw, CANVASDATA canvasdata, BOOLEAN reuse);
void pedigree_increase_generations(INT delta);
void pedigree_toggle_mode(void);

/* scan.c */
INDISEQ full_name_scan(CString sts);
INDISEQ name_fragment_scan(CString sts);
INDISEQ refn_scan(CString sts);
INDISEQ scan_souce_by_author(CString sts);
INDISEQ scan_souce_by_title(CString sts);

/* screen.c */
void clear_status_display(void);

#if defined(DEADENDS)
/* show.c */
STRING indi_to_ped_fix(NODE indi, INT len);
#endif

/* swap.c */
BOOLEAN swap_children(RECORD prnt, RECORD frec);
#if defined(DEADENDS)
BOOLEAN reorder_child(RECORD prnt, RECORD frec, bool rfmt);
#else
BOOLEAN reorder_child(RECORD prnt, RECORD frec, RFMT rfmt);
#endif
BOOLEAN swap_families(RECORD);

/* tandem.c */
INT browse_tandem(RECORD *prec1, RECORD *prec2, INDISEQ *pseq);
INT browse_2fam(RECORD *prec1, RECORD *prec2, INDISEQ *pseq);

/* valgdcom.c */
void addmissingkeys (INT);
int check_stdkeys (void);

#if !defined(DEADENDS) /* scan_header is only used by import.c and valgdcom.c */
BOOLEAN scan_header(FILE * fp, TABLE metadatatab, ZSTR * zerr);
#endif
BOOLEAN validate_gedcom(IMPORT_FEEDBACK ifeed, FILE*);
void validate_end_import(void);
INT validate_get_warning_count(void);
INT xref_to_index (STRING);


#endif /* llinesi_h_included */

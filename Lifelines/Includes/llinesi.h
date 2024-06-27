/*
  Functions used inside the liflines module
  The high-level but not curses-specific portion of the program
  screen.h is not included before this header, so UIWIN etc cannot
  be used here (see screeni.h for declarations requiring screen.h).
*/

#ifndef llinesi_h_included
#define llinesi_h_included

typedef struct tag_llrect {
	int top;
	int bottom;
	int left;
	int right;
} *LLRECT;

/* ask to ensure user got to see the indi */
typedef int CONFIRMQ;
#define DOCONFIRM 1
#define NOCONFIRM 0

struct tag_import_feedback;
#ifndef IMPORT_FEEDBACK_type_defined
typedef struct tag_import_feedback *IMPORT_FEEDBACK;
#define IMPORT_FEEDBACK_type_defined
#endif

struct tag_export_feedback;

/* add.c */
RecordIndexEl *add_family_by_edit(RecordIndexEl *sprec1, RecordIndexEl *sprec2,
				  RecordIndexEl *chrec, bool rfmt);
RecordIndexEl *add_indi_by_edit(bool rfmt);
bool add_indi_no_cache(GNode *);
CString get_unresolved_ref_error_string(int count);
GNode *prompt_add_child(GNode *child, GNode *fam, bool rfmt);
bool prompt_add_spouse(RecordIndexEl *spouse, RecordIndexEl *fam, bool conf);

/* advedit.c */
void advanced_person_edit(GNode *);
void advanced_family_edit(GNode *);

/* ask.c */
RecordIndexEl *ask_for_any(CString ttl, ASK1Q ask1);
Sequence *ask_for_indiseq(CString ttl, char ctype, int *prc);

/* browse.c */
RecordIndexEl *choose_any_event(void);
RecordIndexEl *choose_any_other(void);
RecordIndexEl *choose_any_source(void);
int get_chist_len(void);
Sequence *get_chistory_list(void);
int get_vhist_len(void);
Sequence *get_vhistory_list(void);
bool handle_fam_mode_cmds(int c, int * mode);
bool handle_indi_mode_cmds(int c, int * mode);
bool handle_menu_cmds(int c, bool * reuse);
bool handle_scroll_cmds(int c, bool * reuse);
void history_record_change(RecordIndexEl *);
void init_browse_module(void);
void main_browse(RecordIndexEl *, int code);
GNode *my_prompt_add_child(GNode *child, GNode *fam);
void term_browse_module(void);

/* delete.c */
void choose_and_remove_indi(GNode *indi, CONFIRMQ confirmq);
void choose_and_delete_family(void);
bool choose_and_remove_child(RecordIndexEl *irec, RecordIndexEl *frec,
			     bool nolast);
bool choose_and_remove_spouse(RecordIndexEl *irec, RecordIndexEl *frec,
			      bool nolast);
bool choose_and_remove_any_record(RecordIndexEl *rec, CONFIRMQ confirmq);

/* edit.c */
bool edit_family(RecordIndexEl *frec1, bool rfmt);
bool edit_indi(RecordIndexEl *irec1, bool rfmt);

/* lbrowse.c */
int browse_list(RecordIndexEl **prec1, RecordIndexEl **prec2, Sequence **pseq);

/* lines_usage.c */
void print_lines_usage(CString exename);


/* merge.c */
RecordIndexEl *merge_two_indis(GNode *, GNode *, bool);
RecordIndexEl *merge_two_fams(GNode *, GNode *);

/* miscutls.c */
void key_util(void);
void show_database_stats(void);
#if !defined(DEADENDS)
void who_is_he_she(void);
#endif
void sighand_cursesui(int sig);
void sighand_cmdline(int sig);

/* newrecs.c */
RecordIndexEl *edit_add_event(void);
RecordIndexEl *edit_add_other(void);
RecordIndexEl *edit_add_source(void);
bool edit_any_record(RecordIndexEl *rec, bool rfmt);
bool edit_event(RecordIndexEl *rec, bool rfmt);
bool edit_other(RecordIndexEl *rec, bool rfmt);
bool edit_source(RecordIndexEl *rec, bool rfmt);

/* pedigree.c */
	/* gedcom view mode */
enum { GDVW_NORMAL, GDVW_EXPANDED, GDVW_TEXT };
	/* data for output canvas */
	/* NB: pedigree will adjust scroll if out of limits */
	struct tag_canvasdata;
		/* callback to output a line */
	typedef void (*PEDLINE)(struct tag_canvasdata * canvas, int x, int y
		, String string, int overflow);
		/* collection of data needed by pedigree */
	typedef struct tag_canvasdata { LLRECT rect; int scroll; void * param;
		PEDLINE line; } *CANVASDATA;
	/* functions */
void pedigree_draw_ancestors(RecordIndexEl *rec, CANVASDATA canvasdata,
			     bool reuse);
void pedigree_draw_descendants(RecordIndexEl *rec, CANVASDATA canvasdata,
			       bool reuse);
void pedigree_draw_gedcom(RecordIndexEl *rec, int gdvw, CANVASDATA canvasdata,
			  bool reuse);
void pedigree_increase_generations(int delta);
void pedigree_toggle_mode(void);

/* scan.c */
Sequence *full_name_scan(CString sts);
Sequence *name_fragment_scan(CString sts);
Sequence *refn_scan(CString sts);
Sequence *scan_souce_by_author(CString sts);
Sequence *scan_souce_by_title(CString sts);

/* screen.c */
void clear_status_display(void);

#if defined(DEADENDS)
/* show.c */
String indi_to_ped_fix(GNode *indi, int len);
#endif

/* swap.c */
bool swap_children(RecordIndexEl *prnt, RecordIndexEl *frec);
bool reorder_child(RecordIndexEl *prnt, RecordIndexEl *frec, bool rfmt);
bool swap_families(RecordIndexEl *);

/* tandem.c */
int browse_tandem(RecordIndexEl **prec1, RecordIndexEl **prec2, Sequence **pseq);
int browse_2fam(RecordIndexEl **prec1, RecordIndexEl **prec2, Sequence **pseq);

/* valgdcom.c */
void addmissingkeys (int);
int check_stdkeys (void);

#if !defined(DEADENDS) /* scan_header is only used by import.c and valgdcom.c */
bool scan_header(FILE * fp, TABLE metadatatab, ZSTR * zerr);
#endif
bool validate_gedcom(IMPORT_FEEDBACK ifeed, FILE*);
void validate_end_import(void);
int validate_get_warning_count(void);
int xref_to_index (String);


#endif /* llinesi_h_included */

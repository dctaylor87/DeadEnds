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
GNode *add_family_by_edit(GNode *sprec1, GNode *sprec2,
			  GNode *chrec, bool rfmt);
GNode *add_indi_by_edit(bool rfmt);
bool add_indi_no_cache(GNode *);
CString get_unresolved_ref_error_string(int count);
GNode *prompt_add_child(GNode *child, GNode *fam, bool rfmt);
bool prompt_add_spouse(GNode *spouse, GNode *fam, bool conf);

/* advedit.c */
void advanced_person_edit(GNode *);
void advanced_family_edit(GNode *);

/* ask.c */
GNode *ask_for_any(CString ttl, ASK1Q ask1);
Sequence *ask_for_indiseq(CString ttl, char ctype, int *prc);

/* browse.c */
GNode *chooseAnyEvent(void);
GNode *chooseAnyOther(void);
GNode *chooseAnySource(void);
int get_chist_len(void);
Sequence *get_chistory_list(void);
int get_vhist_len(void);
Sequence *get_vhistory_list(void);
bool handle_fam_mode_cmds(int c, int * mode);
bool handle_indi_mode_cmds(int c, int * mode);
bool handle_menu_cmds(int c, bool * reuse);
bool handle_scroll_cmds(int c, bool * reuse);
void history_record_change(GNode *);
void init_browse_module(void);
void main_browse(GNode *, int code);
GNode *my_prompt_add_child(GNode *child, GNode *fam);
void term_browse_module(void);

/* delete.c */
void chooseAndRemovePerson(GNode *indi, CONFIRMQ confirmq);
bool chooseAndRemoveChild(GNode *irec, GNode *frec,
			     bool nolast);
bool chooseAndRemoveSpouse(GNode *irec, GNode *frec,
			      bool nolast);
bool chooseAndRemoveAnyRecord(GNode *rec, CONFIRMQ confirmq);

/* edit.c */
bool edit_family(GNode *frec1, bool rfmt);
bool edit_indi(GNode *irec1, bool rfmt);

/* lbrowse.c */
int browse_list(GNode **prec1, GNode **prec2, Sequence **pseq);

/* lines_usage.c */
void print_lines_usage(CString exename);


/* merge.c */
GNode *merge_two_indis(GNode *, GNode *, bool);
GNode *merge_two_fams(GNode *, GNode *);

/* miscutls.c */
void key_util(void);
void show_database_stats(Database *);
#if !defined(DEADENDS)
void who_is_he_she(void);
#endif
void sighand_cursesui(int sig);
void sighand_cmdline(int sig);

/* newrecs.c */
GNode *edit_add_event(void);
GNode *edit_add_other(void);
GNode *edit_add_source(void);
bool edit_any_record(GNode *rec, bool rfmt);
bool edit_event(GNode *rec, bool rfmt);
bool edit_other(GNode *rec, bool rfmt);
bool edit_source(GNode *rec, bool rfmt);

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
void pedigree_draw_ancestors(GNode *rec, CANVASDATA canvasdata,
			     bool reuse);
void pedigree_draw_descendants(GNode *rec, CANVASDATA canvasdata,
			       bool reuse);
void pedigree_draw_gedcom(GNode *rec, int gdvw, CANVASDATA canvasdata,
			  bool reuse);
void pedigree_increase_generations(int delta);
void pedigree_toggle_mode(void);

/* scan.c */
Sequence *fullNameScan(CString sts, Database*);
Sequence *nameFragmentScan(CString sts, Database*);
Sequence *refnScan(CString sts, Database*);
Sequence *scanSourceByAuthor(CString sts, Database*);
Sequence *scanSourceByTitle(CString sts, Database*);

/* screen.c */
void clear_status_display(void);

/* show.c */
String indi_to_ped_fix(GNode *indi, int len);

/* swap.c */
bool swap_children(GNode *prnt, GNode *frec);
bool reorder_child(GNode *prnt, GNode *frec, bool rfmt);
bool swap_families(GNode *);

/* tandem.c */
int browse_tandem(GNode **prec1, GNode **prec2, Sequence **pseq);
int browse_2fam(GNode **prec1, GNode **prec2, Sequence **pseq);

/* valgdcom.c */
void addmissingkeys (int);
int check_stdkeys (void);

#if !defined(DEADENDS) /* scan_header is only used by import.c and valgdcom.c */
bool scan_header(FILE * fp, HashTable *metadatatab, ZSTR * zerr);
#endif
bool validate_gedcom(IMPORT_FEEDBACK ifeed, FILE*);
void validate_end_import(void);
int validate_get_warning_count(void);
int xref_to_index (String);


#endif /* llinesi_h_included */

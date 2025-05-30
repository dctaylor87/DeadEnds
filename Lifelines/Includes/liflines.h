#ifndef _LIFLINES_H
#define _LIFLINES_H

/*
 UI functions required for doing work with records
 TODO: ARRAYDETAILS probably doesn't belong here (seems curses dependent)
 TODO: rearrange functions 
    -- (they are currently arranged by curses implementation file
	  and they ought to be arranged in some way not related to curses impl.)
*/

/* Types */

/* screen.c types */
/* data used in chooseFromArray_x */
typedef struct tag_array_details {
  String * list; /* original array of choices */
  int cur; /* currently selected choice */
  String * lines; /* lines of details */
  int count; /* how many lines */
  int maxlen; /* size of each line */
  int scroll; /* scroll offset in details */
} *ARRAY_DETAILS;
typedef void (*DETAILFNC)(ARRAY_DETAILS, void *);

/* whether to prompt for new child if none existing */
typedef int PROMPTQ;
#define ALWAYS_PROMPT 0
#define PROMPT_IF_CHILDREN 1

/* Function Prototypes */
/* add.c */
extern void add_new_indi_to_db(GNode *indi0);
extern void add_new_fam_to_db(GNode *fam2, GNode *spouse1, GNode *spouse2, GNode *child);
void add_child_to_fam(GNode *child, GNode *fam, int i);
GNode *add_family_to_db(GNode *spouse1, GNode *spouse2, GNode *child);
void add_spouse_to_fam(GNode *spouse, GNode *fam, SexType sex);
int ask_child_order(GNode *fam, PROMPTQ promptq, bool rfmt);
//String ask_for_indi_key(CString, ASK1Q ask1);
//GNode *ask_for_indi(CString ttl, ASK1Q ask1);

/* ask.c */
bool ask_for_int(CString, int *);
GNode *ask_for_record(CString, int, Database *);
String ask_for_record_key(CString title, CString prompt);
GNode *chooseFromSequence(Sequence *, ASK1Q ask1, CString titl1, CString titln,
			  enum SequenceType type);

/* askgedc.c */
bool ask_for_gedcom(CString mode, CString ttl, String *pfname,
		    String path, String ext, bool picklist);

/* askprogram.c */
void proparrdetails(ARRAY_DETAILS arrdets, void * param);

/* screen.c functions */
int chooseFromArray_x(CString ttl, int count, String* list, DETAILFNC, void *);
int display_list(CString ttl, List *list);

/* selectdb.c */
//bool open_or_create_database(String *dbused);
//bool select_database(String * dbrequested, String * perrmsg);
Database *selectAndOpenDatabase (CString *dbFilename, CString searchPath,
				 Database *oldDatabase, ErrorLog *errorLog);

/* interp-prog.c */
void interp_main (CString sfile, Database *database, String ofile,
		  bool picklist, bool timing);

/* miscutls.c */
void sighand_cursesui(int sig);

extern ErrorLog *globalErrorLog;
#endif /* _LIFLINES_H */

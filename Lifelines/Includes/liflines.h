#ifndef _LIFLINES_H
#define _LIFLINES_H

/*
 UI functions required for doing work with records
 TODO: ARRAYDETAILS probably doesn't belong here (seems curses dependent)
 TODO: rearrange functions 
    -- (they are currently arranged by curses implementation file
	  and they ought to be arranged in some way not related to curses impl.)
*/


#if !defined(DEADENDS)
#include "standard.h"
#include "gedcom.h"

#ifndef _INDISEQ_H
#include "indiseq.h"
#endif

#ifndef INCLUDED_UIPROMPTS_H
#include "uiprompts.h"
#endif

#endif

/* Types */

/* screen.c types */
/* data used in choose_from_array_x */
typedef struct tag_array_details {
  String * list; /* original array of choices */
  INT cur; /* currently selected choice */
  String * lines; /* lines of details */
  INT count; /* how many lines */
  INT maxlen; /* size of each line */
  INT scroll; /* scroll offset in details */
} *ARRAY_DETAILS;
typedef void (*DETAILFNC)(ARRAY_DETAILS, void *);

/* Function Prototypes */
/* add.c */
extern void add_new_indi_to_db(RECORD indi0);
extern void add_new_fam_to_db(NODE fam2, NODE spouse1, NODE spouse2, NODE child);
void add_child_to_fam(NODE child, NODE fam, INT i);
NODE add_family_to_db(NODE spouse1, NODE spouse2, NODE child);
#if defined(DEADENDS)
void add_spouse_to_fam(NODE spouse, NODE fam, SexType sex);
INT ask_child_order(NODE fam, PROMPTQ promptq, bool rfmt);
#else
void add_spouse_to_fam(NODE spouse, NODE fam, INT sex);
INT ask_child_order(NODE fam, PROMPTQ promptq, RFMT rfmt);
#endif
String ask_for_indi_key(CString, ASK1Q ask1);
RECORD ask_for_indi(CString ttl, ASK1Q ask1);

/* ask.c */
RECORD ask_for_fam(CString, CString);
RECORD ask_for_fam_by_key(CString fttl, CString pttl, CString sttl);
FILE *ask_for_input_file (CString mode, CString ttl, String *pfname, String *pfullpath, String path, String ext);
FILE *ask_for_output_file (CString mode, CString ttl, String *pfname, String *pfullpath, String path, String ext);
INDISEQ ask_for_indi_list(CString, bool);
bool ask_for_int(CString, INT *);
RECORD ask_for_record(CString, INT);
String ask_for_record_key(CString title, CString prompt);
RECORD choose_from_indiseq(INDISEQ, ASK1Q ask1, CString titl1, CString titln);

/* askgedc.c */
bool ask_for_gedcom(CString mode, CString ttl, String *pfname, String *pfullpath
	, String path, String ext, bool picklist);

/* askprogram.c */
void proparrdetails(ARRAY_DETAILS arrdets, void * param);

/* screen.c functions */
INT choose_from_array_x(CString ttl, INT count, String* list, DETAILFNC, void *);
INT display_list(CString ttl, LIST list);

/* selectdb.c */
bool open_or_create_database(INT alteration, String *dbused);
bool select_database(String * dbrequested, INT alteration, String * perrmsg);

/* miscutls.c */
void sighand_cursesui(int sig);

#if defined(DEADENDS)
extern ErrorLog *globalErrorLog;
#endif
#endif /* _LIFLINES_H */

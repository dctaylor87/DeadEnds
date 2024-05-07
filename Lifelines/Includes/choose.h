/* choose.h -- these functions are defined in Lifelines/choose.c.

   In LifeLines, they are declared in gedcom.h.  Our (DeadEnds) GEDCOM
   is significantly different AND they are user-interface related, not
   GEDCOM, so they do no really belong in gedcom.h.  Hence, this
   header file. */

extern RECORD choose_child(RECORD irec, RECORD frec, CString msg0, CString msgn, ASK1Q ask1);
extern void choose_and_remove_family(void);
extern RECORD choose_father(RECORD irec, RECORD frec, CString msg0, CString msgn, ASK1Q ask1);
extern RECORD choose_family(RECORD irec, CString msg0, CString msgn, BOOLEAN fams);
extern RECORD choose_mother(RECORD indi, RECORD fam, CString msg0, CString msgn, ASK1Q ask1);
extern RECORD choose_note(RECORD current, CString msg0, CString msgn);
extern RECORD choose_pointer(RECORD current, CString msg0, CString msgn);
extern RECORD choose_source(RECORD current, CString msg0, CString msgn);
extern RECORD choose_spouse(RECORD irec, CString msg0, CString msgn);

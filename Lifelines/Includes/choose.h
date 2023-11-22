/* choose.h -- these functions are defined in Lifelines/choose.c.

   In LifeLines, they are declared in gedcom.h.  Our (DeadEnds) GEDCOM
   is significantly different AND they are user-interface related, not
   GEDCOM, so they do no really belong in gedcom.h.  Hence, this
   header file. */

extern RECORD choose_child(RECORD irec, RECORD frec, STRING msg0, STRING msgn, ASK1Q ask1);
extern void choose_and_remove_family(void);
extern RECORD choose_father(RECORD irec, RECORD frec, STRING msg0, STRING msgn, ASK1Q ask1);
extern RECORD choose_family(RECORD irec, STRING msg0, STRING msgn, BOOLEAN fams);
extern RECORD choose_mother(RECORD indi, RECORD fam, STRING msg0, STRING msgn, ASK1Q ask1);
extern RECORD choose_note(RECORD current, STRING msg0, STRING msgn);
extern RECORD choose_pointer(RECORD current, STRING msg0, STRING msgn);
extern RECORD choose_source(RECORD current, STRING msg0, STRING msgn);
extern RECORD choose_spouse(RECORD irec, STRING msg0, STRING msgn);

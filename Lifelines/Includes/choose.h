/* choose.h -- these functions are defined in Lifelines/choose.c.

   In LifeLines, they are declared in gedcom.h.  Our (DeadEnds) GEDCOM
   is significantly different AND they are user-interface related, not
   GEDCOM, so they do no really belong in gedcom.h.  Hence, this
   header file. */

extern RecordIndexEl *choose_child(RecordIndexEl *irec, RecordIndexEl *frec,
				   CString msg0, CString msgn, ASK1Q ask1);
extern void choose_and_remove_family(void);
extern RecordIndexEl *choose_father(RecordIndexEl *irec, RecordIndexEl *frec,
				    CString msg0, CString msgn, ASK1Q ask1);
extern RecordIndexEl *choose_family(RecordIndexEl *irec, CString msg0,
				    CString msgn, bool fams);
extern RecordIndexEl *choose_mother(RecordIndexEl *indi, RecordIndexEl *fam,
				    CString msg0, CString msgn, ASK1Q ask1);
extern RecordIndexEl *choose_note(RecordIndexEl *current, CString msg0,
				  CString msgn);
extern RecordIndexEl *choose_pointer(RecordIndexEl *current, CString msg0,
				     CString msgn);
extern RecordIndexEl *choose_source(RecordIndexEl *current, CString msg0,
				    CString msgn);
extern RecordIndexEl *choose_spouse(RecordIndexEl *irec, CString msg0,
				    CString msgn);

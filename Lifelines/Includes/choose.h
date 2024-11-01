/* choose.h -- these functions are defined in Lifelines/choose.c.

   In LifeLines, they are declared in gedcom.h.  Our (DeadEnds) GEDCOM
   is significantly different AND they are user-interface related, not
   GEDCOM, so they do no really belong in gedcom.h.  Hence, this
   header file. */

extern RecordIndexEl *chooseChild(RecordIndexEl *irec, RecordIndexEl *frec,
				   CString msg0, CString msgn, ASK1Q ask1);
extern void chooseAndRemoveFamily(void);
extern RecordIndexEl *chooseFather(RecordIndexEl *irec, RecordIndexEl *frec,
				    CString msg0, CString msgn, ASK1Q ask1);
extern RecordIndexEl *chooseFamily(RecordIndexEl *irec, CString msg0,
				    CString msgn, bool fams);
extern RecordIndexEl *chooseMother(RecordIndexEl *indi, RecordIndexEl *fam,
				    CString msg0, CString msgn, ASK1Q ask1);
extern RecordIndexEl *chooseNote(RecordIndexEl *current, CString msg0,
				  CString msgn);
extern RecordIndexEl *choosePointer(RecordIndexEl *current, CString msg0,
				     CString msgn);
extern RecordIndexEl *chooseSource(RecordIndexEl *current, CString msg0,
				    CString msgn);
extern RecordIndexEl *chooseSpouse(RecordIndexEl *irec, CString msg0,
				    CString msgn);

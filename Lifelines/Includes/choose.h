/* choose.h -- these functions are defined in Lifelines/choose.c.

   In LifeLines, they are declared in gedcom.h.  Our (DeadEnds) GEDCOM
   is significantly different AND they are user-interface related, not
   GEDCOM, so they do no really belong in gedcom.h.  Hence, this
   header file. */

extern GNode *chooseChild(GNode *irec, GNode *frec,
			  CString msg0, CString msgn, ASK1Q ask1);
extern void chooseAndRemoveFamily(void);
extern GNode *chooseFather(GNode *irec, GNode *frec,
			   CString msg0, CString msgn, ASK1Q ask1);
extern GNode *chooseFamily(GNode *irec, CString msg0,
			   CString msgn, bool fams);
extern GNode *chooseMother(GNode *indi, GNode *fam,
			   CString msg0, CString msgn, ASK1Q ask1);
extern GNode *chooseNote(GNode *current, CString msg0,
			 CString msgn);
extern GNode *choosePointer(GNode *current, CString msg0,
			    CString msgn);
extern GNode *chooseSource(GNode *current, CString msg0,
			   CString msgn);
extern GNode *chooseSpouse(GNode *irec, CString msg0,
			   CString msgn);

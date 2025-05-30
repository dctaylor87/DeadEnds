/* choose.h -- these functions are defined in Lifelines/choose.c.

   In LifeLines, they are declared in gedcom.h.  Our (DeadEnds) GEDCOM
   is significantly different AND they are user-interface related, not
   GEDCOM, so they do no really belong in gedcom.h.  Hence, this
   header file. */

extern GNode *chooseChild(GNode *irec, GNode *frec,
			  CString msg0, CString msgn, ASK1Q ask1, Database *database);
extern void chooseAndRemoveFamily(void);
extern GNode *chooseFather(GNode *irec, GNode *frec,
			   CString msg0, CString msgn, ASK1Q ask1, Database *database);
extern GNode *chooseFamily(GNode *irec, CString msg0,
			   CString msgn, bool fams, Database *database);
extern GNode *chooseMother(GNode *indi, GNode *fam,
			   CString msg0, CString msgn, ASK1Q ask1, Database *database);
extern GNode *chooseNote(GNode *current, CString msg0,
			 CString msgn, Database *database);
extern GNode *choosePointer(GNode *current, CString msg0,
			    CString msgn, Database *database);
extern GNode *chooseSource(GNode *current, CString msg0,
			   CString msgn, Database *database);
extern GNode *chooseSpouse(GNode *irec, CString msg0,
			   CString msgn, Database *database);

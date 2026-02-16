extern void remove_indi_by_root (GNode *indi, Database *database);
extern bool removeEmptyFamily (GNode *fam, Database *database);
extern bool removeChildFromFamily (GNode *indi, GNode *fam, Database *database);
extern bool removeSpouseFromFamily (GNode *indi, GNode *fam, Database *database);
extern bool remove_fam_record (GNode *frec);
extern bool remove_any_record (GNode *record, Database *database);
extern int num_fam_xrefs (GNode *fam);



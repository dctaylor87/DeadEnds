extern void remove_indi_by_root (GNode *indi, Database *database);
extern bool remove_empty_fam (GNode *fam, Database *database);
extern bool remove_child (GNode *indi, GNode *fam, Database *database);
extern bool remove_spouse (GNode *indi, GNode *fam, Database *database);
extern bool remove_fam_record (RecordIndexEl *frec);
extern bool remove_any_record (RecordIndexEl *record, Database *database);
extern int num_fam_xrefs (GNode *fam);



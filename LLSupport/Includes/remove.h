extern void remove_indi_by_root (GNode *indi, Database *database);
extern BOOLEAN remove_empty_fam (GNode *fam, Database *database);
extern BOOLEAN remove_child (GNode *indi, GNode *fam, Database *database);
extern BOOLEAN remove_spouse (GNode *indi, GNode *fam, Database *database);
extern BOOLEAN remove_fam_record (RECORD frec);
extern BOOLEAN remove_any_record (RECORD record, Database *database);
extern INT num_fam_xrefs (GNode *fam);



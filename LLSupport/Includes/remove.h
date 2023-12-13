extern void remove_indi_by_root (GNode *indi);
extern BOOLEAN remove_empty_fam (GNode *fam);
extern BOOLEAN remove_child (GNode *indi, GNode *fam);
extern BOOLEAN remove_spouse (GNode *indi, GNode *fam);
extern BOOLEAN remove_fam_record (RECORD frec);
extern BOOLEAN remove_any_record (RECORD record);
extern INT num_fam_xrefs (GNode *fam);



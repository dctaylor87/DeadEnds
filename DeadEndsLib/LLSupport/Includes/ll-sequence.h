
extern Sequence *getAllPersons (Database *database);
extern Sequence *getAllFamilies (Database *database);
extern Sequence *getAllSources (Database *database);
extern Sequence *getAllEvents (Database *database);
extern Sequence *getAllOthers (Database *database);
extern Sequence *getAllRefns (Database *database);

extern Sequence *GNodeToSources (GNode *node, Database *database);
extern Sequence *GNodeToNotes (GNode *node, Database *database);
extern Sequence *GNodeToPointers (GNode *node, Database *database);

//extern CString element_key_indiseq (Sequence *seq, int index);
//extern CString element_skey (SequenceEl *el);
//extern CString element_name (SequenceEl *el);
extern Sequence *familyToSpouses (GNode *fam, Database *database);

/* extern for now, but might become static */
//extern Sequence *getAllRecordIndex (Database *database, RecordIndex *index);
extern Sequence *getAllRecordOfType (Database *database, RecordType type);

#define element_skey(el)	((el)->root->key)
#define element_name(el)	((el)->name)

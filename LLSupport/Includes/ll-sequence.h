extern Sequence *getAllPersons (Database *database);
extern Sequence *getAllFamilies (Database *database);
extern Sequence *getAllSources (Database *database);
extern Sequence *getAllEvents (Database *database);
extern Sequence *getAllOthers (Database *database);
extern Sequence *getAllRefns (Database *database);

extern Sequence *GNodeToSources (GNode *node);
extern Sequence *GNodeToNotes (GNode *node);
extern Sequence *GNodeToPointers (GNode *node);

extern CString element_key_indiseq (Sequence *seq, int index);
extern CString element_skey (SequenceEl *el);
extern CString element_name (SequenceEl *el);
extern Sequence *familyToSpouses (GNode *fam, Database *database);

/* for now, but might become static */
extern Sequence *getAllRecordIndex (Database *database, RecordIndex *index);

extern Sequence *getAllPersons (Database *database);
extern Sequence *getAllFamilies (Database *database);
extern Sequence *getAllSources (Database *database);
extern Sequence *getAllEvents (Database *database);
extern Sequence *getAllOthers (Database *database);
extern Sequence *getAllRefns (Database *database);

/* for now, but might become static */
extern Sequence *getAllRecordIndex (Database *database, RecordIndex *index);

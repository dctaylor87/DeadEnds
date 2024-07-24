extern RecordIndexEl *getFirstPersonRecord (Database *database);
extern RecordIndexEl *getFirstFamilyRecord (Database *database);
extern RecordIndexEl *getFirstSourceRecord (Database *database);
extern RecordIndexEl *getFirstEventRecord (Database *database);
extern RecordIndexEl *getFirstOtherRecord (Database *database);

extern RecordIndexEl *getNextPersonRecord (CString key, Database *database);
extern RecordIndexEl *getNextFamilyRecord (CString key, Database *database);
extern RecordIndexEl *getNextSourceRecord (CString key, Database *database);
extern RecordIndexEl *getNextEventRecord (CString key, Database *database);
extern RecordIndexEl *getNextOtherRecord (CString key, Database *database);

extern CString getNewPersonKey (Database *database);
extern CString getNewFamilyKey (Database *database);
extern CString getNewSourceKey (Database *database);
extern CString getNewEventKey (Database *database);
extern CString getNewOtherKey (Database *database);

extern bool isKeyInUse (CString key, Database *database);

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

extern RecordIndexEl *getLastPersonRecord (Database *database);
extern RecordIndexEl *getLastFamilyRecord (Database *database);
//extern RecordIndexEl *getLastSourceRecord (Database *database);
//extern RecordIndexEl *getLastEventRecord (Database *database);
//extern RecordIndexEl *getLastOtherRecord (Database *database);

extern RecordIndexEl *getPreviousPersonRecord (CString key, Database *database);
extern RecordIndexEl *getPreviousFamilyRecord (CString key, Database *database);
//extern RecordIndexEl *getPreviousSourceRecord (CString key, Database *database);
//extern RecordIndexEl *getPreviousEventRecord (CString key, Database *database);
//extern RecordIndexEl *getPreviousOtherRecord (CString key, Database *database);

extern CString getNewPersonKey (Database *database);
extern CString getNewFamilyKey (Database *database);
extern CString getNewSourceKey (Database *database);
extern CString getNewEventKey (Database *database);
extern CString getNewOtherKey (Database *database);

extern RecordIndexEl *getFirstRecord (RecordType type, Database *database);
extern RecordIndexEl *getLastRecord (RecordType type, Database *database);

extern RecordIndexEl *getNextRecord (RecordType type, CString key, Database *database);
extern RecordIndexEl *getPreviousRecord (RecordType type, CString key, Database *database);

extern bool isKeyInUse (CString key, Database *database);

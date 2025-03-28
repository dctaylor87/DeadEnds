extern GNode *getFirstPersonRecord (Database *database);
extern GNode *getFirstFamilyRecord (Database *database);
extern GNode *getFirstSourceRecord (Database *database);
extern GNode *getFirstEventRecord (Database *database);
extern GNode *getFirstOtherRecord (Database *database);

extern GNode *getNextPersonRecord (CString key, Database *database);
extern GNode *getNextFamilyRecord (CString key, Database *database);
extern GNode *getNextSourceRecord (CString key, Database *database);
extern GNode *getNextEventRecord (CString key, Database *database);
extern GNode *getNextOtherRecord (CString key, Database *database);

extern GNode *getLastPersonRecord (Database *database);
extern GNode *getLastFamilyRecord (Database *database);
extern GNode *getLastSourceRecord (Database *database);
extern GNode *getLastEventRecord (Database *database);
extern GNode *getLastOtherRecord (Database *database);

extern GNode *getPreviousPersonRecord (CString key, Database *database);
extern GNode *getPreviousFamilyRecord (CString key, Database *database);
extern GNode *getPreviousSourceRecord (CString key, Database *database);
extern GNode *getPreviousEventRecord (CString key, Database *database);
extern GNode *getPreviousOtherRecord (CString key, Database *database);

extern CString getNewPersonKey (Database *database);
extern CString getNewFamilyKey (Database *database);
extern CString getNewSourceKey (Database *database);
extern CString getNewEventKey (Database *database);
extern CString getNewOtherKey (Database *database);

extern GNode *getFirstRecord (RecordType type, Database *database);
extern GNode *getLastRecord (RecordType type, Database *database);

extern GNode *getNextRecord (RecordType type, CString key, Database *database);
extern GNode *getPreviousRecord (RecordType type, CString key, Database *database);

extern bool isKeyInUse (CString key, Database *database);

/* The following five functions return a new (unused) key of the specified type.
   It is the callers responsibility to use the key or release it.
   Also, the key is in static storage and the caller should save it
   (strsave or similar).  */

extern CString getNewFamilyKey (Database *database);
extern CString getNewPersonKey (Database *database);
extern CString getNewSourceKey (Database *database);
extern CString getNewEventKey (Database *database);
extern CString getNewOtherKey (Database *database);

/* The following five functions release a key of the specified type.
   It is the callers responsibility to have already removed the
   corresponding record from the database.

   Keys must be of the appropriate type, however we accept both with
   and without the surrounding '@'s.  */

extern void releaseFamilyKey (Database *database, CString key);
extern void releasePersonKey (Database *databas, CString key);
extern void releaseSourceKey (Database *database, CString key);
extern void releaseEventKey (Database *database, CString key);
extern void releaseOtherKey (Database *database, CString key);

extern uint64_t xref_max_any (Database *database);
extern CString value_to_xref (CString val);

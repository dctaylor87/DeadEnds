// The following five functions are all needed by the curses interface.

// keynumToPersonRecord -- Get a person record from a database
extern GNode *keynumToPersonRecord(KEYNUM_TYPE keynum, Database *database);
// keynumToFamilyRecord -- Get a family record from a database
extern GNode *keynumToFamilyRecord(KEYNUM_TYPE keynum, Database *database);
// keynumToSourceRecord -- Get a source record from a database
extern GNode *keynumToSourceRecord(KEYNUM_TYPE keynum, Database *database);
// keynumToEventRecord -- Get an event record from a database
extern GNode *keynumToEventRecord(KEYNUM_TYPE keynum, Database *database);
// keynumToOtherRecord -- Get an other record from a database
extern GNode *keynumToOtherRecord(KEYNUM_TYPE keynum, Database *database);

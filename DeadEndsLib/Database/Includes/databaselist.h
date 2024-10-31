// DeadEnds
//
// databaselist.h is the header file for the DatabaseList type.

typedef List DatabaseList;

extern DatabaseList *createDatabaseList(void);
extern void insertInDatabaseList (DatabaseList *list, Database *database);
extern Database *findInDatabaseList (DatabaseList *list, CString name, int *index);

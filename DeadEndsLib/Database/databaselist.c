// DeadEnds
//
// databaselist.c holds the DatabaseList datatype.  A DatabaseList is
// a List whose elements are pointers to Database objects.
//
// Currently there is only one object, databaseList, of type DatabaseList.
// It holds pointers to all the databases currently in memory.
//
// Created by David Taylor

#include <stdint.h>

#include "database.h"
#include "databaselist.h"

// getKey is the get key function for DatabaseLists
static CString getKey (void *element)
{
  return ((Database *)element)->name;
}

// compare is the compare function for DatabaseLists.
static int compare (CString keyA, CString keyB)
{
  ASSERT (keyA && keyB);	/* do we want to allow NULL keys? */
  return strcmp (keyA, keyB);
}

// delete is the delete function for DatabaseLists.
// NOT YET

DatabaseList *createDatabaseList (void)
{
  return createList (getKey, compare, null, true);
}

// insertInDatabaseList adds a Database* to the DatabaseList.
void insertInDatabaseList (DatabaseList *list, Database *database)
{
  int index = -1;

  if (findInList (list, list->getKey(database), &index))
    {
      fatal ("BUG -- two databases have the same name!\n");
      abort();			/* fatal is supposed to abort */
    }
    insertInList (list, database, index);
}

Database *findInDatabaseList (DatabaseList *list, CString name, int *index)
{
  return findInList (list, name, index);
}

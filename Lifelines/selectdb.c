#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>		/* uint64_t */
#include <stdlib.h>		/* strtoull */
#include <errno.h>		/* ERANGE */
#include <limits.h>		/* ULLONG_MAX */
#include <string.h>
#include <alloca.h>

#include "standard.h"
#include "path.h"
#include "hashtable.h"
#include "gnode.h"
#include "database.h"
#include "import.h"
#include "stringtable.h"
#include "options.h"
#include "name.h"
#include "selectdb.h"
#include "feedback.h"
#include "denls.h"
#include "messages.h"

#define SIZE_REKEY_TABLE	1025

/* forward references */
//static bool indexHasStandardKeys (char prefix, RecordIndex *index);
static void mergeDatabases_1 (Database *database, Database *oldDatabase, bool copy);

/* selectAndOpenDatabase -- search along searchPath for dbFilename and import it.
   The resulting database is returned.
   If errors occur, NULL is returned and the errors are recorded in errorLog.
   If oldDatabase is non-NULL, the new records are added to it. */

Database *
selectAndOpenDatabase(CString *dbFilename,
		      CString searchPath,
		      Database *oldDatabase,
		      ErrorLog *errorLog)
{
  CString newName;
  Database *database;

  /* first, we need to find the file */
  newName = resolveFile (*dbFilename, searchPath);
  if (! newName)
    {
      /* not found, try adding '.ged', if not present, and searching again */
      int len = strlen (*dbFilename);
      if ((len >= 4) && eqstr (".ged", &(*dbFilename[len - 4])))
	{
	  Error *error = createError (systemError, *dbFilename, 0,
				      "file not found anywhere along search path");
	  addErrorToLog (errorLog, error);

	  /* file already ends in '.ged' and it was not found, nothing to do. */
	  return NULL;
	}
      char *buffer = alloca (len + 4 + 1);
      strcpy (buffer, *dbFilename);
      strcat (buffer, ".ged");
      newName = resolveFile (buffer, searchPath);
      if (! newName)
	{
	  Error *error = createError (systemError, *dbFilename, 0,
				      "file not found anywhere along search path");
	  addErrorToLog (errorLog, error);

	  /* file not found, nothing to do */
	  return NULL;
	}
    }

  /* newName has the resolved name */
  //  database = gedcomFileToDatabase (newName, errorLog);
  database = getDatabaseFromFile (newName, 0, errorLog);

  if (! database)
    {
      /* there were errors */
      msg_errorlog (errorLog);
      if (saveErrorLog (ImportLog, errorLog))
	msg_error ("Import errors have been saved in %s\n", ImportLog);
      else
	msg_error ("Attempt to save import errors to %s failed\n", ImportLog);
      return NULL;
    }
  if (oldDatabase)
    {
      if (! checkForRefnOverlap (database, oldDatabase, errorLog))
	/* we're trying to do a merge and there is overlap, give the bad news */
	{
	  /* there were errors */
	  msg_errorlog (errorLog);
	  if (saveErrorLog (ImportLog, errorLog))
	    msg_error ("Import errors have been saved in %s\n", ImportLog);
	  else
	    msg_error ("Attempt to save import errors to %s failed\n", ImportLog);
	  return NULL;
	}
    }

  bool standardKeys = databaseHasStandardKeys (database);
  if (! oldDatabase && standardKeys)
    /* we're not doing a merge and we have standard keys -- we are done */
    return (database);

  if (oldDatabase || ! standardKeys)
    /* need to rekey the new database, figure out the starting key values */
    {
      if (! rekeyDatabase (database, oldDatabase, errorLog))
	{
	  /* there were errors */
	  msg_errorlog (errorLog);
	  if (saveErrorLog (ImportLog, errorLog))
	    msg_error ("Import errors have been saved in %s\n", ImportLog);
	  else
	    msg_error ("Attempt to save import errors to %s failed\n", ImportLog);
	  return NULL;
	}
    }

  /* no failures from here on out, update dbFilename -- newName was
     malloc'ed by resolveFile */
  *dbFilename = newName;

  if (! oldDatabase)
    /* there is no old database, the new one has standard keys, return it */
    return (database);

  mergeDatabases_1 (database, oldDatabase, false);
  deleteDatabase (database);
  return (oldDatabase);
}

bool databaseHasStandardKeys (Database *database)
{
  uint64_t last_person = database->db_max_person;
  uint64_t last_family = database->db_max_family;
  uint64_t last_source = database->db_max_source;
  uint64_t last_event = database->db_max_event;
  uint64_t last_other = database->db_max_other;

  FORHASHTABLE (database->recordIndex, element)
    GNode *gnode = (GNode *)element;
    CString key = gnode->key;

    if (key[0] != '@')
      return false;		/* not a key -- should never happen */

    switch (recordType (gnode))
      {
      case GRPerson:
	if (key[1] != 'I')
	  return false;
	break;
      case GRFamily:
	if (key[1] != 'F')
	  return false;
	break;
      case GRSource:
	if (key[1] != 'S')
	  return false;
	break;
      case GREvent:
	if (key[1] != 'E')
	  return false;
	break;
      case GROther:
	if (key[1] != 'X')
	  return false;
	break;
      default:
	return false;		/* should never happen */
      }
    unsigned long long keynum;
    char *endptr = 0;

    keynum = strtoull (&key[2], &endptr, 10);
    if (*endptr != '@')
      return false;
    if ((keynum == ULLONG_MAX) && (errno == ERANGE))
      return false;		/* overflow */

    switch (recordType (gnode))
      {
      case GRPerson:
	if (keynum > last_person)
	  last_person = keynum;
	break;
      case GRFamily:
	if (keynum > last_family)
	  last_family = keynum;
	break;
      case GRSource:
	if (keynum > last_source)
	  last_source = keynum;
	break;
      case GREvent:
	if (keynum > last_event)
	  last_event = keynum;
	break;
      case GROther:
	if (keynum > last_other)
	  last_other = keynum;
	break;
      default:
	return false;		/* should never happen */
      }
  ENDHASHTABLE

  database->db_max_person = last_person;
  database->db_max_family = last_family;
  database->db_max_source = last_source;
  database->db_max_event = last_event;
  database->db_max_other = last_other;

  return true;
}

/* checkForRefnOverlap -- return  true if okay  to continue (no
   overlap), false if need to abort the database merge */

/* checkForRefnOverlay -- return true if no overlap, false if some
   REFNs overlap.  In the event of overlap, the overlapping REFNs are
   reported in errorLog */

bool
checkForRefnOverlap (Database *database, Database *oldDatabase,
		     ErrorLog *errorLog)
{
  /* validate that there is no REFN overlap */
  bool overlap = false;

  if (! database)
    {
      Error *error = createError (usageError, NULL, 0,
				  "first argument to checkForRefnOverlap is NULL");
      addErrorToLog (errorLog, error);
      return false;
    }
  if (! oldDatabase)
    {
      Error *error = createError (usageError, NULL, 0,
				  "second argument to checkForRefnOverlap is NULL");
      addErrorToLog (errorLog, error);
      return false;
    }

  /* if needed for error messages */
  CString baseName = (oldDatabase->name) ? oldDatabase->name : oldDatabase->path;
  CString refn_fmt = "REFN '%s' in database '%s' (key '%s')\n is already present in base database '%s' (key %s)";

  FORHASHTABLE(database->refnIndex, element)
    CString refn = ((RefnIndexEl *)element)->refn;
    CString baseKey = searchRefnIndex (oldDatabase->refnIndex, refn);
    if (baseKey)
      {
	CString dbName = (database->name) ? database->name : database->path;
	CString dbKey = ((RefnIndexEl *)element)->key;

	int len = strlen (refn_fmt) + strlen (baseName) +
	  strlen (baseKey) + strlen (dbName) + strlen(dbKey) - 8 + 1;

	char *refn_msg = alloca(len);

	sprintf (refn_msg, refn_fmt, refn, dbName, dbKey, baseName, baseKey);
	Error *error = createError (gedcomError, database->path, 0, refn_msg);
	setSeverityError (error, warningError);
	addErrorToLog (errorLog, error);
	overlap = true;
      }
  ENDHASHTABLE

  if (overlap == true)
    return false;

  return true;
}

/* rekeyDatabase -- rekey database using standard keys, use
   oldDatabase, if non-NULL, to determine starting values. */

bool
rekeyDatabase (Database *database, Database *oldDatabase, ErrorLog *errorLog)
{
  uint64_t last_indi;
  uint64_t last_fam;
  uint64_t last_even;
  uint64_t last_sour;
  uint64_t last_othr;
  char prefix;

  if (oldDatabase)
    {
      last_indi = oldDatabase->db_max_person;
      last_fam = oldDatabase->db_max_family;
      last_even = oldDatabase->db_max_event;
      last_sour = oldDatabase->db_max_source;
      last_othr = oldDatabase->db_max_other;
    }
  else
    {
      last_indi = 0;
      last_fam = 0;
      last_even = 0;
      last_sour = 0;
      last_othr = 0;
    }

  StringTable *rekeyTable = createStringTable (SIZE_REKEY_TABLE);
  if (! rekeyTable)
    {
      Error *error = createError (systemError, database->path, 0,
				  "unable to rekey due to createStringTable failing");
      addErrorToLog (errorLog, error);
      return false;
    }
  FORHASHTABLE (database->recordIndex, element)
    GNode *root = (GNode *)element;
    CString oldKey = root->key;
    char newKey[25];
    uint64_t newNumber;

    switch (recordType (root))
      {
      case GRPerson:
	prefix = 'I';
	newNumber = ++last_indi;
	break;
      case GRFamily:
	prefix = 'F';
	newNumber = ++last_fam;
	break;
      case GRSource:
	prefix = 'S';
	newNumber = ++last_sour;
	break;
      case GREvent:
	prefix = 'E';
	newNumber = ++last_even;
	break;
      case GROther:
	prefix = 'X';
	newNumber = ++last_othr;
 	break;
      default:
	continue;		/* should never happen! */
      }
    sprintf (newKey, "@%c%llu@", prefix, (unsigned long long)newNumber);
    /* addToStringTable does strsave on both oldKey & newKey */
    addToStringTable (rekeyTable, oldKey, newKey);
  ENDHASHTABLE

  /* now, we have the mappings, let's update all the values */

  /* recordIndex, personIndex, familyIndex, sourceIndex, eventIndex,
     otherIndex all ultimately point to the root GNode and use the key
     within the record.  So, if we update recordIndex, we update them
     all. */
  FORHASHTABLE(database->recordIndex, element)
    GNode *root = ((GNodeListEl *)element)->node;
    CString key = root->key;
    if (key)
      {
	CString newKey = searchStringTable (rekeyTable, key);
	stdfree(root->key);
	root->key = strsave (newKey);
      }
    FORTRAVERSE(root, node)
      if (isKey (node->value))
	{
	  CString newValue = searchStringTable (rekeyTable, node->value);
	  stdfree(node->value);
	  node->value = strsave(newValue);
	}
    ENDTRAVERSE
  ENDHASHTABLE

  /* now, let's do the refnIndex... */
  FORHASHTABLE(database->refnIndex, element)
    RefnIndexEl *elt = (RefnIndexEl *)element;
    CString newKey = searchStringTable (rekeyTable, elt->key);
    stdfree(elt->key);
    elt->key = strsave(newKey);
  ENDHASHTABLE

  /* now for the nameIndex... */
  NameIndex *newIndex = createNameIndex();
  if (! newIndex)
    {
      /* should not happen -- must be low memory or similar */
      Error *error = createError (systemError, database->path, 0,
				  "createNameIndex failed -- ?out of memory?");
      addErrorToLog (errorLog, error);
      return false;
    }
  FORHASHTABLE(database->nameIndex, element)
    NameIndexEl *elt = (NameIndexEl *)element;
    String nameKey = elt->nameKey;
    FORSET(elt->recordKeys, setElement)
      CString newKey = searchStringTable (rekeyTable, setElement);
      insertInNameIndex (newIndex, nameKey, newKey);
    ENDSET
  ENDHASHTABLE

  deleteNameIndex (database->nameIndex);
  database->nameIndex = newIndex;
  /* personRoots and familyRoots are unaffected by rekeying */

  return true;
}

/* mergeDatabases -- does error checking, if everything passes, calls
   mergeDatabases_1 to do the actual merge.  Returns true if the
   databases were merge, returns false if one of the error checks
   failed.

   On failure, database MIGHT be altered (depending on when the
   failure occurred); on success, both database and oldDatabase are
   altered and oldDatabase contains re-keyed entries from the
   database.  On success, it is reasonable to call deleteDatabase on
   database.  */

bool
mergeDatabases (Database *database, Database *oldDatabase, ErrorLog *errorLog)
{
  if (! database)
    {
      Error *error = createError (systemError, "", 0,
				  "mergeDatabases -- called with NULL database");
      addErrorToLog (errorLog, error);
    }
  if (! oldDatabase)
    {
      Error *error = createError (systemError, "", 0,
				  "mergeDatabases -- called with NULL oldDatabase");
      addErrorToLog (errorLog, error);
    }
  if (! checkForRefnOverlap (database, oldDatabase, errorLog))
    return false;

  if (! rekeyDatabase (database, oldDatabase, errorLog))
    return false;		/* unlikely, typically means malloc failed */

  mergeDatabases_1 (database, oldDatabase, true);
  return true;
}

/* mergeDatabases_1 -- merge everything in database into oldDatabase */

static void
mergeDatabases_1 (Database *database, Database *oldDatabase, bool copy)
{
  /* copy the records: database --> oldDatabase */
  FORHASHTABLE(database->recordIndex, element)
    GNode *root = (GNode *)element;
    if (copy)
      root = copyGNodes (root, true, true);
    switch (recordType (root))
      {
      case GRPerson:
	addToRecordIndex (oldDatabase->recordIndex, root);
	insertInRootList (oldDatabase->personRoots, root);
	for (GNode *name = NAME(root);
	     name && eqstr (name->tag, "NAME");
	     name = name->sibling)
	  {
	    if (name->value)
	      {
		String nameKey = nameToNameKey (name->value);
		insertInNameIndex (oldDatabase->nameIndex, nameKey, root->key);
	      }
	  }
	break;
      case GRFamily:
	addToRecordIndex (oldDatabase->recordIndex, root);
	insertInRootList (oldDatabase->familyRoots, root);
	break;
      case GRSource:
      case GREvent:
      case GROther:
	addToRecordIndex (oldDatabase->recordIndex, root);
      default:
      }
  ENDHASHTABLE

  /* database->refnIndex has already been checked for non-existent
     values, empty values, and overlap with oldDatabase, so we only
     need to propagate the values. */
  FORHASHTABLE (database->refnIndex, element)
    RefnIndexEl *elt = (RefnIndexEl *) element;
    addToRefnIndex (oldDatabase->refnIndex, elt->refn, elt->key);
  ENDHASHTABLE
}

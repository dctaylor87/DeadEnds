#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <inttypes.h>		/* needed by PRIdKEYNUM */
#include <stdint.h>

#include "ll-standard.h"

#include "refnindex.h"
#include "gnode.h"
#include "recordindex.h"
#include "stringtable.h"
#include "nameindex.h"
#include "name.h"
#include "database.h"

#include "ll-database.h"

// keynumTo...Record -- Given a keynum, get a record of the specified type.
// These five functions assume keys are all of the form <letter><number>,
// with <letter> determined by type and <number> NOT having any leading zeros.
// Theyt exist because the curses interface calls them.

// keynumToPersonRecord -- Get a person record from a database
GNode *keynumToPersonRecord(KEYNUM_TYPE keynum, Database *database)
{
  char key[MAXKEYWIDTH + 3];
  if ((keynum > MAXKEYNUMBER) || (keynum <= 0))
    return NULL;

  sprintf (key, "@I" FMT_KEYNUM, keynum);
  return keyToPersonRecord (key, database);
}

// keynumToFamilyRecord -- Get a family record from a database
GNode *keynumToFamilyRecord(KEYNUM_TYPE keynum, Database *database)
{
  char key[MAXKEYWIDTH + 3];
  if ((keynum > MAXKEYNUMBER) || (keynum <= 0))
    return NULL;

  sprintf (key, "@F" FMT_KEYNUM, keynum);
  return keyToFamilyRecord (key, database);
}

// keynumToSourceRecord -- Get a source record from a database
GNode *keynumToSourceRecord(KEYNUM_TYPE keynum, Database *database)
{
  char key[MAXKEYWIDTH + 3];
  if ((keynum > MAXKEYNUMBER) || (keynum <= 0))
    return NULL;

  sprintf (key, "@S" FMT_KEYNUM, keynum);
  return keyToSourceRecord (key, database);
}

// keynumToEventRecord -- Get an event record from a database
GNode *keynumToEventRecord(KEYNUM_TYPE keynum, Database *database)
{
  char key[MAXKEYWIDTH + 3];
  if ((keynum > MAXKEYNUMBER) || (keynum <= 0))
    return NULL;

  sprintf (key, "@E" FMT_KEYNUM, keynum);
  return keyToPersonRecord (key, database);
}

// keynumToOtherRecord -- Get an other record from a database
GNode *keynumToOtherRecord(KEYNUM_TYPE keynum, Database *database)
{
  char key[MAXKEYWIDTH + 3];
  if ((keynum > MAXKEYNUMBER) || (keynum <= 0))
    return NULL;

  sprintf (key, "@X" FMT_KEYNUM, keynum);
  return keyToPersonRecord (key, database);
}

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "standard.h"
#include "denls.h"
#include "sys_inc.h"

#include "refnindex.h"
#include "gnode.h"
#include "database.h"
#include "xref.h"

static RecordIndexEl *genericGetNextRecord (CString key, RecordIndex *index);

RecordIndexEl *getFirstPersonRecord (Database *database)
{
  int bucket_index;
  int element_index;

  return firstInHashTable (database->personIndex, &bucket_index, &element_index);
}

RecordIndexEl *getFirstFamilyRecord (Database *database)
{
  int bucket_index;
  int element_index;

  return firstInHashTable (database->familyIndex, &bucket_index, &element_index);
}

RecordIndexEl *getFirstSourceRecord (Database *database)
{
  int bucket_index;
  int element_index;

  return firstInHashTable (database->sourceIndex, &bucket_index, &element_index);
}

RecordIndexEl *getFirstEventRecord (Database *database)
{
  int bucket_index;
  int element_index;

  return firstInHashTable (database->eventIndex, &bucket_index, &element_index);
}

RecordIndexEl *getFirstOtherRecord (Database *database)
{
  int bucket_index;
  int element_index;

  return firstInHashTable (database->otherIndex, &bucket_index, &element_index);
}

RecordIndexEl *getFirstRecord (RecordType type, Database *database)
{
  switch (type)
    {
    case GRPerson:
      return getFirstPersonRecord (database);
    case GRFamily:
      return getFirstFamilyRecord (database);
    case GRSource:
      return getFirstSourceRecord (database);
    case GREvent:
      return getFirstEventRecord (database);
    case GROther:
      return getFirstOtherRecord (database);
    default:
      return NULL;
    }
}

RecordIndexEl *getNextPersonRecord (CString key, Database *database)
{
  return genericGetNextRecord (key, database->personIndex);
}

RecordIndexEl *getNextFamilyRecord (CString key, Database *database)
{
  return genericGetNextRecord (key, database->familyIndex);
}

RecordIndexEl *getNextSourceRecord (CString key, Database *database)
{
  return genericGetNextRecord (key, database->sourceIndex);
}

RecordIndexEl *getNextEventRecord (CString key, Database *database)
{
  return genericGetNextRecord (key, database->eventIndex);
}

RecordIndexEl *getNextOtherRecord (CString key, Database *database)
{
  return genericGetNextRecord (key, database->otherIndex);
}

RecordIndexEl *getNextRecord (RecordType type, CString key, Database *database)
{
  switch (type)
    {
    case GRPerson:
      return getNextPersonRecord (key, database);
    case GRFamily:
      return getNextFamilyRecord (key, database);
    case GRSource:
      return getNextSourceRecord (key, database);
    case GREvent:
      return getNextEventRecord (key, database);
    case GROther:
      return getNextOtherRecord (key, database);
    default:
      return NULL;
    }
}

static RecordIndexEl *genericGetNextRecord (CString key, RecordIndex *index)
{
  int bucket_index = 0;
  int element_index = 0;
  void *detail;

  if (! key)
    return firstInHashTable (index, &bucket_index, &element_index);

  detail = detailSearchHashTable (index, key, &bucket_index, &element_index);
  if (! detail)
    /* need to log this error somehow!  maybe additional argument? */
    return NULL;		/* caller gave us a bad key */

  /* null above is not found -- caller gave us a bad key, null here
     means exhausted -- have gone through all the elements, no more to
     be returned. */
  return nextInHashTable (index, &bucket_index, &element_index);
}

bool isKeyInUse (CString key, Database *database)
{
  if (searchHashTable (database->personIndex, key))
    return true;
  if (searchHashTable (database->familyIndex, key))
    return true;
  if (searchHashTable (database->sourceIndex, key))
    return true;
  if (searchHashTable (database->eventIndex, key))
    return true;
  if (searchHashTable (database->otherIndex, key))
    return true;

  return false;
}

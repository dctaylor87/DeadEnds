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

static GNode *genericGetNextRecord (CString key, RecordIndex *index, RecordType type);

static GNode *
getFirstRecord_PF (RootList *rootList);

static GNode *
getLastRecord_PF (RootList *rootList);

static GNode *
getNextRecord_PF (CString key, RootList *rootList);

static GNode *
getPreviousRecord_PF (CString key, RootList *rootList);

static GNode *
genericPreviousRecord (CString key, RecordIndex *index, RecordType type);

GNode *getFirstPersonRecord (Database *database)
{
  return getFirstRecord_PF (database->personRoots);
}

GNode *getFirstFamilyRecord (Database *database)
{
  return getFirstRecord_PF (database->familyRoots);
}

GNode *getFirstSourceRecord (Database *database)
{
  return genericGetNextRecord (NULL, database->recordIndex, GRSource);
}

GNode *getFirstEventRecord (Database *database)
{
  return genericGetNextRecord (NULL, database->recordIndex, GREvent);
}

GNode *getFirstOtherRecord (Database *database)
{
  return genericGetNextRecord (NULL, database->recordIndex, GROther);
}

GNode *getFirstRecord (RecordType type, Database *database)
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

GNode *getNextPersonRecord (CString key, Database *database)
{
  return getNextRecord_PF (key, database->personRoots);
}

GNode *getNextFamilyRecord (CString key, Database *database)
{
  return getNextRecord_PF (key, database->familyRoots);
}

GNode *getNextSourceRecord (CString key, Database *database)
{
  return genericGetNextRecord (key, database->recordIndex, GRSource);
}

GNode *getNextEventRecord (CString key, Database *database)
{
  return genericGetNextRecord (key, database->recordIndex, GREvent);
}

GNode *getNextOtherRecord (CString key, Database *database)
{
  return genericGetNextRecord (key, database->recordIndex, GROther);
}

GNode *getNextRecord (RecordType type, CString key, Database *database)
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

static GNode *genericGetNextRecord (CString key, RecordIndex *index, RecordType type)
{
  int bucket_index = 0;
  int element_index = 0;
  GNode *new;
  void *detail;

  if (! key)
    new = (GNode *)firstInHashTable (index, &bucket_index, &element_index);
  else
    {
      detail = detailSearchHashTable (index, key, &bucket_index, &element_index);
      if (! detail)
	return NULL;		/* caller gave us a bad key */
      new = (GNode *)nextInHashTable (index, &bucket_index, &element_index);
    }
  while (new && (recordType(new) != type))
    new = nextInHashTable (index, &bucket_index, &element_index);

  /* if new is NULL, we've exhausted the index, if non-NULL, we found one.  */
  return (new);
}

/* get first record for Persons and Families */

static GNode *
getFirstRecord_PF (RootList *rootList)
{
  if (! rootList)
    return NULL;

  if (lengthList(rootList) == 0)
    return NULL;		/* list empty */

  sortList (rootList);
  GNode *new = getListElement (rootList, 0);

  return (new);
}

GNode *getLastPersonRecord (Database *database)
{
  return getLastRecord_PF (database->personRoots);
}

GNode *getLastFamilyRecord (Database *database)
{
  return getLastRecord_PF (database->familyRoots);
}

GNode *getLastSourceRecord (Database *database)
{
  return genericPreviousRecord (NULL, database->recordIndex, GRSource);
}

GNode *getLastEventRecord (Database *database)
{
  return genericPreviousRecord (NULL, database->recordIndex, GREvent);
}

GNode *getLastOtherRecord (Database *database)
{
  return genericPreviousRecord (NULL, database->recordIndex, GROther);
}

/* get last record for Persons and Families */

static GNode *
getLastRecord_PF (RootList *rootList)
{
  if (! rootList)
    return NULL;

  if (lengthList(rootList) == 0)
    return NULL;		/* list empty */

  sortList (rootList);
  GNode *new = getListElement (rootList, lengthList(rootList) - 1);

  return (new);
}

GNode *getLastRecord (RecordType type, Database *database)
{
  switch (type)
    {
    case GRPerson:
      return getLastRecord_PF(database->personRoots);
    case GRFamily:
      return getLastRecord_PF(database->familyRoots);
    case GRSource:
      return genericPreviousRecord (NULL, database->recordIndex, GRSource);
    case GREvent:
      return genericPreviousRecord (NULL, database->recordIndex, GREvent);
    case GROther:
      return genericPreviousRecord (NULL, database->recordIndex, GROther);

    default:			/* should never happen */
      return NULL;
    }
}

/* get next record for Persons and Families */

static GNode *
getNextRecord_PF (CString key, RootList *rootList)
{
  sortList (rootList);

  int index;
  GNode *current = findInList (rootList, key, &index);
  if ((! current) || (index < 0) || (index >= lengthList(rootList)))
    {
      /* XXX report error somehow XXX */
      return NULL;
    }
  if (index == lengthList (rootList))
    return NULL;		/* at end of list, exhausted, no more */
  GNode *new = getListElement (rootList, index + 1);

  return (new);
}

GNode *getPreviousPersonRecord (CString key, Database *database)
{
  return getPreviousRecord_PF (key, database->personRoots);
}

GNode *getPreviousFamilyRecord (CString key, Database *database)
{
  return getPreviousRecord_PF (key, database->familyRoots);
}

/* get previous record for Persons and Families */

static GNode *
getPreviousRecord_PF (CString key, RootList *rootList)
{
  sortList (rootList);

  int index;
  GNode *current = findInList (rootList, key, &index);
  if ((! current) || (index < 0) || (index >= lengthList(rootList)))
    {
      /* XXX report error somehow XXX */
      return NULL;
    }
  if (index == 0)
    return NULL;		/* at end of list, exhausted, no more */
  GNode *new = getListElement (rootList, index - 1);

  return (new);
}

static GNode *
genericPreviousRecord (CString key, RecordIndex *index, RecordType type)
{
  int bucket_index;
  int element_index;
  GNode *new;
  void *detail;

  if (! key)
    new = (GNode *)lastInHashTable (index, &bucket_index, &element_index);
  else
    {
      detail = detailSearchHashTable (index, key, &bucket_index, &element_index);
      if (! detail)
	return NULL;		/* caller gave us a bad key */
      new = (GNode *)previousInHashTable (index, &bucket_index, &element_index);
    }
  while (new && (recordType(new) != type))
    new = previousInHashTable (index, &bucket_index, &element_index);

  /* if new is NULL, we've exhausted the index, if non-NULL, we found one.  */
  return (new);
}

GNode *
getPreviousSourceRecord (CString key, Database *database)
{
  return genericPreviousRecord (key, database->recordIndex, GRSource);
}

GNode *
getPreviousEventRecord (CString key, Database *database)
{
  return genericPreviousRecord (key, database->recordIndex, GREvent);
}

GNode *
getPreviousOtherRecord (CString key, Database *database)
{
  return genericPreviousRecord (key, database->recordIndex, GROther);
}

GNode *
getPreviousRecord (RecordType type, CString key, Database *database)
{
  switch (type)
    {
    case GRPerson:
      return getPreviousPersonRecord (key, database);
    case GRFamily:
      return getPreviousFamilyRecord (key, database);
    case GRSource:
      return getPreviousSourceRecord (key, database);
    case GREvent:
      return getPreviousEventRecord (key, database);
    case GROther:
      return getPreviousOtherRecord (key, database);
    default:
      return NULL;		/* bad call */
    }
}

bool isKeyInUse (CString key, Database *database)
{
  if (searchHashTable (database->recordIndex, key))
    return true;

  return false;
}

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

static RecordIndexEl *
getFirstRecord_PF (RecordIndex *recordIndex, RootList *rootList);

static RecordIndexEl *
getLastRecord_PF (RecordIndex *recordIndex, RootList *rootList);

static RecordIndexEl *
getNextRecord_PF (CString key, RecordIndex *recordIndex, RootList *rootList);

static RecordIndexEl *
getPreviousRecord_PF (CString key, RecordIndex *recordIndex, RootList *rootList);

static RecordIndexEl *genericLastRecord (RecordIndex *index);

RecordIndexEl *getFirstPersonRecord (Database *database)
{
  return getFirstRecord_PF (database->personIndex, database->personRoots);
}

RecordIndexEl *getFirstFamilyRecord (Database *database)
{
  return getFirstRecord_PF (database->familyIndex, database->familyRoots);
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
  return getNextRecord_PF (key, database->personIndex, database->personRoots);
}

RecordIndexEl *getNextFamilyRecord (CString key, Database *database)
{
  return getNextRecord_PF (key, database->familyIndex, database->familyRoots);
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

/* get first record for Persons and Families */

static RecordIndexEl *
getFirstRecord_PF (RecordIndex *recordIndex, RootList *rootList)
{
  if (! recordIndex || ! rootList)
    return NULL;

  if (lengthList(rootList) == 0)
    return NULL;		/* list empty */

  sortList (rootList);
  GNode *new = getListElement (rootList, 0);

  /* sadly, we need the record,, not the root node, so we are not done. */
  RecordIndexEl *record = searchHashTable (recordIndex, new->key);
  if (! record)
    {
      /* XXX report error -- database corrupt -- record in RootList
       but not in RecordIndex XXX*/
      return NULL;
    }
  return (record);
}

RecordIndexEl *getLastPersonRecord (Database *database)
{
  return getLastRecord_PF (database->personIndex, database->personRoots);
}

RecordIndexEl *getLastFamilyRecord (Database *database)
{
  return getLastRecord_PF (database->familyIndex, database->familyRoots);
}

RecordIndexEl *getLastSourceRecord (Database *database)
{
  return genericLastRecord (database->sourceIndex);
}

RecordIndexEl *getLastEventRecord (Database *database)
{
  return genericLastRecord (database->eventIndex);
}

RecordIndexEl *getLastOtherRecord (Database *database)
{
  return genericLastRecord (database->otherIndex);
}

/* get last record for Persons and Families */

static RecordIndexEl *
getLastRecord_PF (RecordIndex *recordIndex, RootList *rootList)
{
  if (! recordIndex || ! rootList)
    return NULL;

  if (lengthList(rootList) == 0)
    return NULL;		/* list empty */

  sortList (rootList);
  GNode *new = getListElement (rootList, lengthList(rootList) - 1);

  /* sadly, we need the record,, not the root node, so we are not done. */
  RecordIndexEl *record = searchHashTable (recordIndex, new->key);
  if (! record)
    {
      /* XXX report error -- database corrupt -- record in RootList
       but not in RecordIndex XXX*/
      return NULL;
    }
  return (record);
}

RecordIndexEl *getLastRecord (RecordType type, Database *database)
{
  switch (type)
    {
    case GRPerson:
      return getLastRecord_PF(database->personIndex, database->personRoots);
    case GRFamily:
      return getLastRecord_PF(database->familyIndex, database->familyRoots);
    case GRSource:
      return genericLastRecord (database->sourceIndex);
    case GREvent:
      return genericLastRecord (database->eventIndex);
    case GROther:
      return genericLastRecord (database->otherIndex);

    default:			/* should never happen */
      return NULL;
    }
}

static RecordIndexEl *genericLastRecord (RecordIndex *index)
{
  int bucketIndex = 0;
  int elementIndex = 0;
  return (RecordIndexEl *)lastInHashTable (index, &bucketIndex, &elementIndex);
}

/* get next record for Persons and Families */

static RecordIndexEl *
getNextRecord_PF (CString key, RecordIndex *recordIndex, RootList *rootList)
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

  /* sadly, we need the record, not the root node, so we are not done. */
  RecordIndexEl *record = searchHashTable (recordIndex, new->key);
  if (! record)
    {
      /* XXX report error -- database corrupt -- record in RootList
	 but not in RecordIndex XXX */
      return NULL;
    }
  return (record);
}

RecordIndexEl *getPreviousPersonRecord (CString key, Database *database)
{
  return getPreviousRecord_PF (key, database->personIndex, database->personRoots);
}

RecordIndexEl *getPreviousFamilyRecord (CString key, Database *database)
{
  return getPreviousRecord_PF (key, database->familyIndex, database->familyRoots);
}

/* get previous record for Persons and Families */

static RecordIndexEl *
getPreviousRecord_PF (CString key, RecordIndex *recordIndex, RootList *rootList)
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

  /* sadly, we need the record, not the root node, so we are not done. */
  RecordIndexEl *record = searchHashTable (recordIndex, new->key);
  if (! record)
    {
      /* XXX report error -- database corrupt -- record in RootList
	 but not in RecordIndex XXX */
      return NULL;
    }
  return (record);
}

static RecordIndexEl *
genericPreviousRecord (CString key, RecordIndex *recordIndex)
{
  int bucketIndex;
  int elementIndex;

  if (! key)
    return lastInHashTable (recordIndex, &bucketIndex, &elementIndex);

  RecordIndexEl *record = detailSearchHashTable (recordIndex, key, &bucketIndex, &elementIndex);
  if (! record)
    return NULL;		/* bad input */

  return previousInHashTable (recordIndex, &bucketIndex, &elementIndex);
}

RecordIndexEl *
getPreviousSourceRecord (CString key, Database *database)
{
  return genericPreviousRecord (key, database->sourceIndex);
}

RecordIndexEl *
getPreviousEventRecord (CString key, Database *database)
{
  return genericPreviousRecord (key, database->eventIndex);
}

RecordIndexEl *
getPreviousOtherRecord (CString key, Database *database)
{
  return genericPreviousRecord (key, database->otherIndex);
}

RecordIndexEl *
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

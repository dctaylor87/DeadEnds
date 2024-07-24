/* XXX the files ll-node-seq.c and ll-indiseq.c should be merged into
   here.

   The file ll-node-seq.c is a subset of ll-indiseq.c with three
   non-static functions renamed.

   ll-node-seq.c's claim to fame is that it compiles, while
   ll-indiseq.c does not compile. XXX */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>
#include "standard.h"
#include "llnls.h"

#include "gnode.h"
#include "recordindex.h"
#include "refnindex.h"
#include "database.h"
#include "sequence.h"

#include "ll-sequence.h"
#include "py-set.h"

/* XXX Questions:
   When the caller is done with the sequence, what is currently done?
   Create static function; Collapse the existing five functions into calls to it
   Should the keys be sorted?
   Should there be a getAllRefns?  XXX */

Sequence *
getAllPersons (Database *database)
{
  return getAllRecordIndex (database, database->personIndex);
}

Sequence *
getAllFamilies (Database *database)
{
  return getAllRecordIndex (database, database->familyIndex);
}

Sequence *
getAllSources (Database *database)
{
  return getAllRecordIndex (database, database->sourceIndex);
}

Sequence *
getAllEvents (Database *database)
{
  return getAllRecordIndex (database, database->eventIndex);
}

Sequence *
getAllOthers (Database *database)
{
  return getAllRecordIndex (database, database->otherIndex);
}

Sequence *
getAllRecordIndex (Database *database, RecordIndex *index)
{
  Sequence *seq = createSequence (database);
  int bucket = 0;
  int element = 0;
  RecordIndexEl *record = 0;

  for (record = firstInHashTable (index, &bucket, &element);
       record;
       record = nextInHashTable (index, &bucket, &element))
    appendToSequence (seq, record->root->key, NULL);

  if (lengthSequence (seq) <= 0)
    {
      deleteSequence (seq);
      seq = 0;
    }

  return seq;
}

Sequence *
getAllRefns (Database *database)
{
  Sequence *seq = createSequence (database);
  int bucket = 0;
  int element = 0;
  RefnIndexEl *refnelt = 0;

  for (refnelt = firstInHashTable (database->refnIndex, &bucket, &element);
       refnelt;
       refnelt = nextInHashTable (database->refnIndex, &bucket, &element))
    appendToSequence (seq, refnelt->refn, NULL);

  if (lengthSequence (seq) <= 0)
    {
      deleteSequence (seq);
      seq = 0;
    }

  return seq;
}

/* familyToSpouses -- Create sequence of spouses of family */

Sequence *
familyToSpouses (GNode *fam, Database *database)
{
  Sequence *seq=0;
  int len = 0;

  if (!fam)
    return NULL;
  seq = createSequence (database);

  FORFAMSPOUSES(fam, spouse, database)
    len++;
    appendToSequence(seq, __key, spouse);
  ENDFAMSPOUSES

  if (! len) {
    deleteSequence(seq);
    seq=NULL;
  }
  return seq;
}

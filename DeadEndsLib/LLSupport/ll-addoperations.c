#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "standard.h"
#include "denls.h"
#include "sys_inc.h"

#include "hashtable.h"
#include "refnindex.h"
#include "gnode.h"
#include "database.h"
#include "ll-addoperations.h"

/* forward reference */
static bool
addOrUpdateRecordInDatabase (RecordType type, GNode *node, Database *database);

/* If a person is not in the database, add them; if they are, then
   update them */
bool
addOrUpdatePersonInDatabase (GNode *person, Database *database)
{
  return addOrUpdateRecordInDatabase (GRPerson, person, database);
}

/* If a family is not in the database, add them; if they are, then
   update them */
bool
addOrUpdateFamilyInDatabase (GNode *family, Database *database)
{
  return addOrUpdateRecordInDatabase (GRFamily, family, database);
}

/* If a source is not in the database, add it; if it is, then
   update it */
bool
addOrUpdateSourceInDatabase (GNode *source, Database *database)
{
  return addOrUpdateRecordInDatabase (GRSource, source, database);
}

/* If an event is not in the database, add it; if it is, then
   update it */

bool
addOrUpdateEventInDatabase (GNode *event, Database *database)
{
  return addOrUpdateRecordInDatabase (GREvent, event, database);
}

/* If an Other record is not in the database, add it; if it it, then
   update it */
bool
addOrUpdateOtherInDatabase (GNode *other, Database *database)
{
  return addOrUpdateRecordInDatabase (GROther, other, database);
}

static bool
addOrUpdateRecordInDatabase (RecordType type,
			     GNode *node,
			     Database *database)
{
  return storeRecord (database, node);
}

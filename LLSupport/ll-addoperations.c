#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "standard.h"
#include "llnls.h"
#include "sys_inc.h"

#include "refnindex.h"
#include "gnode.h"
#include "database.h"
#include "ll-addoperations.h"

static bool
AddOrUpdateRecordInDatabase (RecordType type, GNode *node, Database *database);

/* If a person is not in the database, add them; if they are, then
   update them */
bool
AddOrUpdatePersonInDatabase (GNode *person, Database *database)
{
  return AddOrUpdateRecordInDatabase (GRPerson, person, database);
}

/* If a family is not in the database, add them; if they are, then
   update them */
bool
AddOrUpdateFamilyInDatabase (GNode *family, Database *database)
{
  return AddOrUpdateRecordInDatabase (GRFamily, family, database);
}

/* If a source is not in the database, add it; if it is, then
   update it */
bool
AddOrUpdateSourceInDatabase (GNode *source, Database *database)
{
  return AddOrUpdateRecordInDatabase (GRSource, source, database);
}

/* If an event is not in the database, add it; if it is, then
   update it */

bool
AddOrUpdateEventInDatabase (GNode *event, Database *database)
{
  return AddOrUpdateRecordInDatabase (GREvent, event, database);
}

/* If an Other record is not in the database, add it; if it it, then
   update it */
bool
AddOrUpdateOtherInDatabase (GNode *other, Database *database)
{
  return AddOrUpdateRecordInDatabase (GROther, other, database);
}

static bool
AddOrUpdateRecordInDatabase (RecordType type,
			     GNode *node,
			     Database *database)
{
  ErrorLog *errorLog;

  return storeRecord (database, node, -1, errorLog);
}

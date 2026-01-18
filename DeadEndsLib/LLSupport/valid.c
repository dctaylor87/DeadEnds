#include "standard.h"
#include "path.h"
#include "errors.h"
#include "gnode.h"
#include "gedcom.h"
#include "hashtable.h"
#include "stringtable.h"
#include "database.h"
#include "validate.h"

#include "valid.h"
#include "denls.h"
#include "options.h"

bool
validate_new_record (GNode *new, GNode *orig, RecordType ntype,
		  Database *database, CString *pmsg)
{
  ErrorLog *errorLog = createErrorLog();
  bool retval;

  retval = validateNewRecord (new, orig, ntype, database, errorLog);

  if (retval)
    {
      deleteErrorLog (errorLog);
      return (retval);		/* no errors */
    }

  /* UI is responsible for setting ImmportLog during startup */
  retval = saveErrorLog(ImportLog, errorLog);

  static char buffer[MAXPATHLENGTH + 200];

  if (lengthList (errorLog) == 1)
    {
      /* XXX insert code to retrieve first error and set pmsg XXX */
      Error *error = getFirstListElement (errorLog);
      snprintf (buffer, sizeof (buffer), "%s %s error: %s",
		strErrorSeverity(error), strErrorType(error), error->message);
      *pmsg = buffer;
    }
  else if (! retval)
    {
      /* unable to save error messages, give bad news */
      snprintf (buffer, sizeof (buffer),
		_("Errors occurred. Attempt to save errors in '%s' failed"),
		ImportLog);
      *pmsg = buffer;
    }
  else
    {
      snprintf (buffer, sizeof (buffer),
		_("Errors occurred.  For details, see '%s'"), ImportLog);
      *pmsg = buffer;
    }
  deleteErrorLog (errorLog);
  return false;
}

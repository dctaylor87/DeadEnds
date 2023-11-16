#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdio.h>

#include "standard.h"
#include "parse.h"
#include "pnode.h"
#include "interp.h"
#include "functiontable.h"
#include "recordindex.h"
#include "database.h"

#include "deadends.h"

/* forward references */

static int execute_script (CString script);

/* start of code */

/* array of scripts seen on the command line, in the order seen */
static const char **script_array;

/* how many scripts we've seen */
static int nscripts = 0;

/* so that one time initialization is only run once */
static int initialized = 0;

/* deadend_register_script -- we record or register the sciripts seen on
   the command line.  We don't try to be fancy and optimize the calls
   to allocate space for the script names as typically the user will
   only specify one script or possibly two.

   TODO: figure out how to do script specific arguments and options. */

int
deadend_register_script (CString script)
{
  const char **new_array;

  if (nscripts)
    new_array = (const char **)realloc ((void *)script_array, (nscripts + 1) * sizeof (char *));
  else
    new_array = (const char **)malloc (sizeof (char *));

  if (! new_array)
    return (-1);		/* malloc / realloc failed! */

  new_array[nscripts] = script;
  script_array = new_array;
  nscripts++;
  return (0);
}

/* deadend_execute_scripts -- execute the scripts in the order specified
   on the command line.

   continue_on_failure -- if true and a script fails, we continue to
   the next script, otherwise we return with the failure status.

   TODO: make continue on failure script specific.

   FUTURE TODO: figure out how to pass arguments and options to the
   script -- something better than reading stdin for arguments. */

int
deadend_execute_scripts (int continue_on_failure)
{
  int saved_status = 0;

  if (! initialized)
    {
      deadend_script_init ();
      initialized = 1;
    }

  for (int cur_script = 0; cur_script < nscripts; cur_script++)
    {
      int status = execute_script (script_array[cur_script]);
      if (status < 0)
	{
	  if (! continue_on_failure)
	    return (status);
	  else
	    saved_status = status;
	}
    }
  return (saved_status);	/* most recent failure, if any, otherwise zero */
}

static int
execute_script (CString report_name)
{
  // parse a DeadEnds script
  parseProgram (report_name, DEADENDS_search_path);

  //  Create a PNProcCall node to call the main procedure with
  currentProgramFileName = report_name;
  currentProgramLineNumber = 1;
  PNode *pnode = procCallPNode("main", null);

  //  Call the main procedure.
  SymbolTable *symbolTable = createSymbolTable();

  PValue returnPvalue;
  interpret(pnode, symbolTable, &returnPvalue);

  return (0);			/* XXX should interpret returnPvalue XXX */
}

/* XXX consider making this static XXX */
void
deadend_script_init(void)
{
}

/* this will likely be either renamed or merged into another file. */

#include <ansidecl.h>
#include <stdint.h>

#include <time.h>

#include "standard.h"
#include "list.h"
#include "sequence.h"
#include "gnode.h"
#include "parse.h"
#include "path.h"
#include "pnode.h"
#include "interp.h"
#include "errors.h"

#include "messages.h"
#include "zstr.h"
#include "translat.h"
#include "xlat.h"
#include "ask.h"
#include "rptui.h"
#include "feedback.h"
#include "locales.h"
#include "denls.h"
#include "lloptions.h"
#include "dateprint.h"
#include "readwrite.h"
#include "codesets.h"
#include "liflines.h"		/* interp_main */

static int
interp_program (String proc, int nargs, void **args, CString sfile,
		Database *database, String ofile, bool picklist);
#if 0
static bool
interpret_prog (PNode *begin, SymbolTable *stab);
#endif
static CString findProgram (CString script, CString searchPath);

static void
print_report_duration (int duration, int uiduration);

/* interp_main -- Interpreter main proc

   sfile:    [IN]  program files
   ofile:    [IN]  output file - can be NULL
   picklist: [IN]  show user list of existing reports ?
   timing:   [IN]  show report elapsed time info ? */

void
interp_main (CString sfile, Database *database, String ofile, bool picklist, bool timing)
{
  time_t begin = time(NULL);
  int elapsed;
  int uitime;
  int ranit = 0;
  programDebugging = false;		/* clear report debug flag */

  rptui_init ();		/* clear ui time counter */
  rptlocale ();
  ranit = interp_program ("main", 0, NULL, sfile, database, ofile, picklist);
  uilocale ();
  elapsed = time(NULL) - begin;
  uitime = rptui_elapsed ();

  if (ranit && timing)
    print_report_duration (elapsed, uitime);
}

/* interp_program -- Interpret LifeLines program
   proc:     [IN]  proc to call
   nargs:    [IN]  number of arguments
   args:     [IN]  arguments
   sfile:    [IN]  program (script) file
   database  [IN]  database to use
   ofile:    [IN]  output file - can be NULL
   picklist: [IN]  show user list of existing reports ?
 
   returns 0 if it didn't actually run (eg, not found, or no report picked) */

static int
interp_program (String proc, int nargs, void **args, CString sfile,
		Database *database, String ofile, bool picklist)
{
  SymbolTable *stab = NULL;
  int ranit=0;
  String programsdir = getdeoptstr("DEPROGRAMS", NULL);

  if (! programsdir)		/* fall back, most LL scripts work with DE */
    programsdir = getdeoptstr("LLPROGRAMS", ".");

  String fullpath = 0;
  if (sfile)
    {
      fullpath = findProgram (sfile, programsdir);
      if (! fullpath)
	{
	  llwprintf(_("Report not found: %s"), sfile);
	  return false;
	}
      /* findProgram found it!  */
    }
  else
    {
      /* Caller did not provide a script name, ask the user for one */
      String fname = null;
      if (! rptui_ask_for_program(DEREADTEXT, _(qSwhatrpt), &fname,
				  programsdir, ".ll", picklist))
	{
	  llwprintf(_("Report not found: %s"), sfile);
	  return false;
	}
      /* rptui_ask_for_program found it!  */
      sfile = fname;
    }

  /* give interpreter its chance at initialization */
  initializeInterpreter(database);

  ErrorLog *errorLog = createErrorLog ();
  parseProgram (sfile, programsdir, errorLog);
  //programParsing = true;

  if (Perrors)
    {
      msg_error(_("Program contains errors.\n"));

      if (lengthList (errorLog) > 0)
	msg_errorlog (errorLog);
      deleteList(errorLog);
      errorLog = 0;

      goto interp_program_exit;
    }

  /* Open output file if name is provided */

  if (ofile) {
    String errorMessage = null;
    if (! setScriptOutputFile (ofile, false, &errorMessage)) {
      msg_error ("%s %s", errorMessage, ofile);
      goto interp_program_exit;
    }
  }
  if (Poutfp) setbuf(Poutfp, NULL);

  /* Link arguments to parameters in symbol table */

  curFileName = "internal";
  curLine = 1;
  PNode *pnode = procCallPNode ("main", null);

  //stab = createSymbolTable ();
  Context *context = createContext (database);

  /* Interpret top procedure */
  ranit = 1;
  programParsing = false;
  programRunning = true;
  Perrors = 0;
  msg_output(MSG_STATUS, _("Program is running..."));
  ranit = interpret(pnode, context, null);

  /* Clean up and return */

  programRunning = false;
  finishInterpreter(); /* includes 5 sec delay if errors on-screen */
  closeFile (context->file);

 interp_program_exit:

  if (stab) {
    //remove_symtab(stab);
    deleteHashTable (stab);
    stab = NULL;
  }

  xl_free_adhoc_xlats();

  return ranit;
}

#if 0
/* interpret_prog -- execute a report program */

static bool
interpret_prog (PNode *begin, SymbolTable *stab)
{
  PValue *dummy=0;
  enum InterpType rtn = interpret(begin, stab, &dummy);

  delete_pvalue(dummy);
  dummy=0;

  switch (rtn)
    {
    case InterpOkay:
    case InterpReturn:
      msg_output(MSG_INFO, _("Program was run successfully.\n"));
      return true;
    default:
#if defined(DEADENDS)
      /* XXX verify this XXX */
      msg_output(MSG_STATUS, _("Program was not run because of errors.\n"));
#else
      if (rpt_cancelled) {
	progmessage(MSG_STATUS, _("Program was cancelled.\n"));
      } else
	progmessage(MSG_STATUS, _("Program was not run because of errors.\n"));
#endif
      return false;
    }
}
#endif

/* report_duration -- print report duration */

static void
print_report_duration (int duration, int uiduration)
{
  ZSTR zt1=approx_time(duration-uiduration), zt2=approx_time(uiduration);
  llwprintf(_("\nReport duration %s (ui duration %s)\n")
	    , zs_str(zt1), zs_str(zt2));
  zs_free(&zt1);
  zs_free(&zt2);
}

/* findProgram -- given a script name, search for it, both as given
   and with a ".ll" extension.  If found, return the full path,
   otherwise return null.

   NOTE: the path returned by reesolveFile, and hence by us, is
   malloc'ed and needs to be freed by our caller.  */

static
CString findProgram (CString script, CString searchPath)
{
  /* first, try script file name as provided */
  CString fullpath = resolveFile (script, searchPath);

  if (! fullpath)
    {
      /* script file not found as given, check for ".ll" suffix
	 If not present, add it and try again.  */

      int len = strlen (script);
      if (nestr (&script[len - 3], ".ll"))
	{
	  char buffer[len + 4];
	  strcpy (buffer, script);
	  strcpy (buffer + len, ".ll");
	  /* now, try with the ".ll" suffix */
	  fullpath = resolveFile (buffer, searchPath);
	}
      if (! fullpath)
	return null;
    }
  return (fullpath);
}

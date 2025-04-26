/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/
/*=============================================================
 * main.c -- Main program of DeadEnds
 * Copyright(c) 1992-95 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 07 Aug 93
 *   2.3.6 - 02 Oct 93    3.0.0 - 11 Oct 94
 *   3.0.1 - 11 Oct 93    3.0.2 - 01 Jan 95
 *   3.0.3 - 02 Jul 96
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "denls.h"

#include "llgettext.h"
#include "locales.h"
#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "refnindex.h"
#include "gnode.h"
#include "rfmt.h"
#include "sequence.h"
#include "hashtable.h"
#include "database.h"		/* currentDatabase */
#include "errors.h"
#include "ask.h"
#include "feedback.h"
#include "uiio.h"
#include "llinesi.h"
#include "liflines.h"
#include "messages.h"
#include "xlat.h"
#include "readwrite.h"
#include "stringtable.h"
#include "options.h"
#include "codesets.h"
#include "ll-list.h"
#include "gstrings.h"
#include "de-strings.h"
#include "lloptions.h"
#include "interp.h"
#include "signals.h"
#include "errlog.h"
#include "version.h"
#include "init.h"
#include "parse-args.h"

/* for parser debugging */
extern int yydebug;

#ifdef HAVE_GETOPT
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#endif /* HAVE_GETOPT */

#if defined(HAVE_PYTHON)
#include "llpy-externs.h"
#endif

//#if !defined(NUMBER_EXARGS_BUCKETS)
//#define NUMBER_EXARGS_BUCKETS	17
//#endif
//int num_exargs_buckets = NUMBER_EXARGS_BUCKETS;

static CString optString = "adkns:tu:x:o:zC:I:p:Pvh?";

/*********************************************
 * required global variables
 *********************************************/

bool debugmode = false;     /* no signal handling, so we can get coredump */
bool opt_nocb  = false;     /* no cb. data is displayed if TRUE */
bool keyflag   = true;      /* show key values */
int alldone       = 0;         /* completion flag */
bool traditional = true;    /* use traditional family rules */
bool showusage = false;     /* show usage */
bool showversion = false;   /* show version */

Database *currentDatabase = 0;
/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
//static void load_usage(void);
//static void main_db_notify(String db, bool opening);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==================================
 * main -- Main routine of DeadEnds
 *================================*/
int
main (int argc, char **argv)
{
	char * msg;
	int c;
	bool ok=false;
	//	bool python_interactive = false;
	String dbrequested=NULL; /* database (path) requested */
	//	List *exprogs=NULL;
	//	TABLE exargs=NULL;
	//	String progout=NULL;
	String configfile=0;
	String crashlog=NULL;
	int i=0;
	//	bool have_python_scripts = false;
	UIIO *saved_uiio = NULL;

	current_uiio = uiio_curses;

	/* DEADENDS: init_arch is just 'return 0', init_stdlib is
	   misnamed -- it initializes some btree stuff used by the LL
	   on disk database. */
#if !defined(DEADENDS)
	/* initialize all the low-level library code */
	init_stdlib();
#endif

#if HAVE_SETLOCALE
	/* initialize locales */
	setlocale(LC_ALL, "");
#endif /* HAVE_SETLOCALE */
	
#if ENABLE_NLS
	/* setup gettext translation */
	/* NB: later we may revise locale dir or codeset
	based on user settings */
	ll_bindtextdomain(PACKAGE, LOCALEDIR);
	textdomain(PACKAGE);
#endif

	save_original_locales();
	//load_usage();

	/* Parse Command-Line Arguments */
	parseArguments (argc, argv, optString);

prompt_for_db:

	/* catch any fault, so we can close database */
	if (!debugmode)
	{
		set_signals(sighand_cursesui);
	}
	/* developer wants to drive without seatbelt! */
	else
	{
		stdstring_hardfail();
		/* yydebug = 1; */
	}

	set_displaykeys(keyflag);

	/* initialize options & misc. stuff */
	llgettext_set_default_localedir(LOCALEDIR);
#if defined(DEADENDS)
	if (! init_lifelines_global(configfile, &msg)) {
		llwprintf("%s", msg);
		goto finish;
	}
#else
	if (!init_lifelines_global(configfile, &msg, &main_db_notify)) {
		llwprintf("%s", msg);
		goto finish;
	}
#endif
	/* setup crashlog in case init_screen fails (eg, bad menu shortcuts) */
	crashlog = getdeoptstr(crashlog_optname, NULL);
	if (! crashlog)
		crashlog = crashlog_default;

	crash_setcrashlog(crashlog);

	/* do we need curses?  If we're running a script or the Python
	   interpreter, don't bother... */

	bool runningInterpreter = false;
	if (exprogs || have_python_scripts || python_interactive)
	  {
	    current_uiio = uiio_stdio;
	    runningInterpreter = true;
	  }
	if (! uiio_init (current_uiio))
	  goto finish;

	if (! uiio_pre_database_init (current_uiio, runningInterpreter))
	  goto finish;

	c = argc - optind;
	if (c > 1) {
		showusage = true;
		goto usage;
	}

	if (exargs) {
		set_cmd_options(exargs);
		releaseHashTable(exargs);
		exargs = 0;
	}
	/* Open database, prompting user if necessary */
	{
	  if (!alldone && c>0)
	    dbrequested = strsave(argv[optind]);
	  else
	    {
	      fprintf (stderr, "%s: database argument is required\n", ProgName);
	      print_usage();
	      exit(1);
	    }

	  ErrorLog *errorLog = createErrorLog ();
	  currentDatabase = selectAndOpenDatabase (&dbrequested, NULL, NULL, errorLog);
	  if (! currentDatabase)
	    {
	      showErrorLog (errorLog);
	      deleteList ((List *)errorLog);
	      alldone = 0;
	      goto finish;
	    }
	}
	/* give interpreter its turn at initialization */
	//initializeInterpreter(currentDatabase);

	/* Start Program */
	if (!init_lifelines_postdb()) {
		llwprintf("%s", _(qSbaddb));
		goto finish;
	}

	uiio_post_database_init (current_uiio);

	if (exprogs) {
		bool picklist = false;
		bool timing = false;
		int len = lengthList (exprogs);
		for (int ndx = 0; ndx < len; ndx++) {
		  CString prog = getListElement (exprogs, ndx);
		  interp_main (prog, currentDatabase, progout, picklist, timing);
		}
		//interp_main(exprogs, currentDatabase, progout, picklist, timing);
		deleteList(exprogs);
	} else if (have_python_scripts) {
#if HAVE_PYTHON
		int status = llpy_execute_scripts (false);
		if (status < 0)
			ok = false;
		else
			ok = true;
#else
		fprintf (stderr, "Sorry, but Python support is not available in this version of DeadEnds\n");
		ok=false;
#endif
	} else if (python_interactive) {
#if HAVE_PYTHON
		int status = llpy_python_interactive ();
		if ((status == 0))
			ok=true;
		else
			ok=false;
#else
		fprintf (stderr, "Sorry, but Python support is not available in this version of DeadEnds\n");
		ok=false;
#endif
	} else {
		alldone = 0;
		while (!alldone)
			main_menu();
	}

	term_show_module();
	term_browse_module();
	ok=true;

finish:
	/* we free this not because we care so much about these tiny amounts
	of memory, but to ensure we have the memory management right */
	/* strfree frees memory & nulls pointer */
	strfree(&dbrequested);
#if !defined(DEADENDS)
	strfree(&readpath_file);
	shutdown_interpreter();
#endif
#if HAVE_PYTHON
	llpy_python_terminate ();
#endif
	close_lifelines();
	uiio_shutdown_ui(current_uiio, !ok);
	if (alldone == 2)
		goto prompt_for_db; /* changing databases */
	termlocale();

usage:
	/* Display Version and/or Command-Line Usage Help */
	if (showversion) { print_version(ProgName); }
	if (showusage) puts(usage_summary);

	/* Exit */
	return !ok;
}

/* Finnish language support modifies the soundex codes for names, so
 * a database created with this support is not compatible with other
 * databases. 
 *
 * define FINNISH for Finnish Language support
 *
 * define FINNISHOPTION to have a runtime option -F which will enable
 * 	  	Finnish language support, but the name indices will all be
 *      wrong if you make modifications whilst in the wrong mode.
 */

#if 0
static void
load_usage (void)
{
	usage_summary = _(qSusgNorm);
}
#endif
/*===============================================
 * print_usage -- display program help/usage
 *  displays to stdout
 *=============================================*/
void
print_usage (void)
{
  print_lines_usage(ProgName, true);
}

#if !defined(DEADENDS)
/*==================================================
 * main_db_notify -- callback called whenever a db is
 *  opened or closed
 * Created: 2002/06/16, Perry Rapp
 *================================================*/
static void
main_db_notify (String db, bool opening)
{
	/* store name away for reporting in case of crash later */
	if (opening)
		crash_setdb(db);
	else
		crash_setdb("");
}
#endif

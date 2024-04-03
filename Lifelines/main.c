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
 * main.c -- Main program of LifeLines
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
#include "llnls.h"

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
#include "uiprompts.h"
#include "feedback.h"
#include "llinesi.h"
#include "errors.h"
#include "liflines.h"
#include "messages.h"
#include "screen.h"
#include "readwrite.h"
#include "options.h"
#include "stringtable.h"
#include "codesets.h"
#include "ll-list.h"
#include "gstrings.h"
#include "de-strings.h"
#include "uiio.h"

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

/*********************************************
 * external variables (no header)
 *********************************************/

extern INT winx, winy;

/*********************************************
 * required global variables
 *********************************************/


static STRING usage_summary = "";      /* usage string */
BOOLEAN debugmode = FALSE;     /* no signal handling, so we can get coredump */
BOOLEAN opt_nocb  = FALSE;     /* no cb. data is displayed if TRUE */
BOOLEAN keyflag   = TRUE;      /* show key values */
INT alldone       = 0;         /* completion flag */
extern BOOLEAN progrunning;
extern BOOLEAN progparsing;
extern INT     progerror;
BOOLEAN traditional = TRUE;    /* use traditional family rules */
BOOLEAN showusage = FALSE;     /* show usage */
BOOLEAN showversion = FALSE;   /* show version */

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void load_usage(void);
static void main_db_notify(STRING db, BOOLEAN opening);
static void parse_arg(const char * optarg, char ** optname, char **optval);
static void print_usage(void);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==================================
 * main -- Main routine of LifeLines
 *================================*/
int
main (int argc, char **argv)
{
	char * msg;
	int c;
	BOOLEAN ok=FALSE;
	BOOLEAN python_interactive = FALSE;
	STRING dbrequested=NULL; /* database (path) requested */
	STRING dbused=NULL; /* database (path) found */
	INT alteration=0;
	LIST exprogs=NULL;
	TABLE exargs=NULL;
	STRING progout=NULL;
	BOOLEAN graphical=TRUE;
	STRING configfile=0;
	STRING crashlog=NULL;
	int i=0;
	BOOLEAN have_python_scripts = FALSE;

	current_uiio = uiio_curses;

	/* DEADENDS: init_arch is just 'return 0', init_stdlib is
	   misnamed -- it initializes some btree stuff used by the LL
	   on disk database. */
#if !defined(DEADENDS)
	/* initialize all the low-level platform code */
	init_arch();

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
	load_usage();

	/* handle conventional arguments --version and --help */
	/* needed for help2man to synthesize manual pages */
	for (i=1; i<argc; ++i) {
		if (!strcmp(argv[i], "--version")
			|| !strcmp(argv[i], "-v")) {
			print_version("llines");
			return 0;
		}
		if (!strcmp(argv[i], "--help")
			|| !strcmp(argv[i], "-h")
			|| !strcmp(argv[i], "-?")) {
			print_usage();
			return 0;
		}
	}

	/* Parse Command-Line Arguments */
	opterr = 0;	/* turn off getopt's error message */
	while ((c = getopt(argc, argv, "adkntu:x:o:zC:I:p:Pvh?")) != -1) {
		switch (c) {
		case 'a':	/* debug allocation */
#if defined(DEADENDS)
		  logAllocations(true);
#else
			alloclog = TRUE;
#endif
			break;
		case 'd':	/* debug = no signal catchers */
			debugmode = TRUE;
			break;
		case 'k':	/* don't show key values */
			keyflag = FALSE;
			break;
		case 'n':	/* use non-traditional family rules */
			traditional = FALSE;
			break;
#if !defined(DEADENDS)		/* XXX not currently supported by DeadEnds XXX */
		case 't': /* show lots of trace statements for debugging */
			prog_trace = TRUE;
			break;
#endif
		case 'u': /* specify screen dimensions */
			sscanf(optarg, SCN_INT "," SCN_INT, &winx, &winy);
			break;
		case 'x': /* execute program */
			if (!exprogs) {
				exprogs = create_list2(LISTDOFREE);
			}
			push_list(exprogs, strdup(optarg ? optarg : ""));
			break;
		case 'I': /* program arguments */
			{
				STRING optname=0, optval=0;
				parse_arg(optarg, &optname, &optval);
				if (optname && optval) {
					if (!exargs) {
						exargs = create_table_str();
					}
					insert_table_str(exargs, optname, optval);
				}
				strfree(&optname);
				strfree(&optval);
			}
			break;
		case 'o': /* output directory */
			progout = optarg;
			break;
		case 'z': /* nongraphical box */
			graphical = FALSE;
			break;
		case 'C': /* specify config file */
			configfile = optarg;
			break;
		case 'P':	/* python interactive */
			python_interactive = TRUE;
			break;
		case 'p':
#if HAVE_PYTHON
			llpy_register_script (optarg);
#endif
			have_python_scripts = TRUE;
			break;
		case 'v': /* show version */
			showversion = TRUE;
			goto usage;
			break;
		case 'h': /* show usage */
		case '?': /* show usage */
			showusage = TRUE;
			showversion = TRUE;
			goto usage;
			break;
		}
	}

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
	if (!init_lifelines_global(configfile, &msg, &main_db_notify)) {
		llwprintf("%s", msg);
		goto finish;
	}
	/* setup crashlog in case init_screen fails (eg, bad menu shortcuts) */
	crashlog = getlloptstr("CrashLog_llines", NULL);
	if (!crashlog) { crashlog = "CrashLog_llines.log"; }
	crash_setcrashlog(crashlog);

	/* do we need curses?  If we're running a script or the Python
	   interpreter, don't bother... */
	if (! exprogs && ! have_python_scripts && ! python_interactive) {
	        /* start (n)curses and create windows */
		char errmsg[512];
		if (!init_screen(errmsg, sizeof(errmsg)/sizeof(errmsg[0])))
		{
			endwin();
			fprintf(stderr, "%s", errmsg);
			goto finish;
		}
		set_screen_graphical(graphical);
	}
	initializeInterpreter(); /* give interpreter its turn at initialization */

	c = argc - optind;
	if (c > 1) {
		showusage = TRUE;
		goto usage;
	}

	if (exargs) {
		set_cmd_options(exargs);
		release_table(exargs);
		exargs = 0;
	}
	/* Open database, prompting user if necessary */
	if (1) {
		STRING errmsg=0;
		if (!alldone && c>0) {
			dbrequested = strsave(argv[optind]);
		} else {
			strupdate(&dbrequested, "");
		}
		if (!select_database(&dbrequested, alteration, &errmsg)) {
			if (errmsg) {
				llwprintf("%s", errmsg);
			}
			alldone = 0;
			goto finish;
		}
	}

	/* Start Program */
	if (!init_lifelines_postdb()) {
		llwprintf("%s", _(qSbaddb));
		goto finish;
	}
	if (!int_codeset[0]) {
		msg_info("%s", _("Warning: database codeset unspecified"));
	} else if (!transl_are_all_conversions_ok()) {
		msg_info("%s", _("Warning: not all conversions available"));
	}

	init_show_module();
	init_browse_module();
#if 0				/* this will be initialized if we use it -p/-P */
#if defined(HAVE_PYTHON)
	llpy_init ();
#endif
#endif

	if (exprogs) {
		BOOLEAN picklist = FALSE;
		BOOLEAN timing = FALSE;
		interp_main(exprogs, progout, picklist, timing);
		destroy_list(exprogs);
	} else if (have_python_scripts) {
#if HAVE_PYTHON
		int status = llpy_execute_scripts (FALSE);
		if (status < 0)
			ok = FALSE;
		else
			ok = TRUE;
#else
		fprintf (stderr, "Sorry, but Python support is not available in this version of Lifelines\n");
		ok=FALSE;
#endif
	} else if (python_interactive) {
#if HAVE_PYTHON
		int status = llpy_python_interactive ();
		if ((status == 0))
			ok=TRUE;
		else
			ok=FALSE;
#else
		fprintf (stderr, "Sorry, but Python support is not available in this version of Lifelines\n");
		ok=FALSE;
#endif
	} else {
		alldone = 0;
		while (!alldone)
			main_menu();
	}
	term_show_module();
	term_browse_module();
	ok=TRUE;

finish:
	/* we free this not because we care so much about these tiny amounts
	of memory, but to ensure we have the memory management right */
	/* strfree frees memory & nulls pointer */
	strfree(&dbrequested);
	strfree(&dbused);
#if !defined(DEADENDS)
	strfree(&readpath_file);
	shutdown_interpreter();
#endif
#if HAVE_PYTHON
	llpy_python_terminate ();
#endif
	close_lifelines();
	shutdown_ui(!ok);
	if (alldone == 2)
		goto prompt_for_db; /* changing databases */
	termlocale();

usage:
	/* Display Version and/or Command-Line Usage Help */
	if (showversion) { print_version("llines"); }
	if (showusage) puts(usage_summary);

	/* Exit */
	return !ok;
}
/*==================================
 * parse_arg -- Break argument into name & value
 *  eg, parse_arg("main_indi=I3", &a, &b)
 *   yields a="main_indi" and b="I3"
 *  (a & b are newly allocated from heap)
 *================================*/
static void
parse_arg (const char * optarg, char ** optname, char **optval)
{
	const char * ptr;
	*optname = *optval = 0;
	for (ptr = optarg; *ptr; ++ptr) {
		if (*ptr == '=') {
			char * namebuff = 0;
			char * valbuff = 0;
			INT namelen = ptr - optarg;
			if (!namelen)
				return;
			namebuff = (char *)malloc(namelen+1);
			llstrncpy(namebuff, optarg, namelen+1, 0);
			*optname = namebuff;
			valbuff = strdup(ptr+1);
			*optval = valbuff;
			return;

		}
	}
}
/*===================================================
 * shutdown_ui -- Do whatever is necessary to close GUI
 * Created: 2001/11/08, Perry Rapp
 *=================================================*/
void
shutdown_ui (BOOLEAN pause)
{
	term_screen();
	if (pause) /* if error, give user a second to read it */
		sleep(1);
	/* Terminate Curses UI */
	if (!isendwin())
		endwin();
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

static void
load_usage (void)
{
	usage_summary = _(qSusgNorm);
}
/*===============================================
 * print_usage -- display program help/usage
 *  displays to stdout
 *=============================================*/
static void
print_usage (void)
{
#ifdef WIN32
	char * exename = "Lines";
#else
	char * exename = "llines";
#endif
	print_lines_usage(exename);
}
/*==================================================
 * main_db_notify -- callback called whenever a db is
 *  opened or closed
 * Created: 2002/06/16, Perry Rapp
 *================================================*/
static void
main_db_notify (STRING db, BOOLEAN opening)
{
	/* store name away for reporting in case of crash later */
	if (opening)
		crash_setdb(db);
	else
		crash_setdb("");
}

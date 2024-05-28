/* 
   Copyright (c) 2000-2007 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * llexec.c -- Frontend code for lifelines report generator
 * Copyright(c) 2002-2007 by Perry Rapp; all rights reserved
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#ifdef HAVE_LOCALE_H
#include <locale.h>
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
#include "database.h"		/* currentDatabase */
#include "uiprompts.h"
#include "feedback.h"
//#include "ui.h"
#include "llinesi.h"
//#include "version.h"
#include "errors.h"
#include "liflines.h"
#include "messages.h"
#include "readwrite.h"
#include "options.h"
#include "stringtable.h"
#include "codesets.h"
#include "ll-list.h"
#include "gstrings.h"
#include "de-strings.h"
#include "uiio.h"
#include "lloptions.h"
#include "interp.h"
#include "signals.h"

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
//#include "uiio.h"

#if !defined(NUMBER_EXARGS_BUCKETS)
#define NUMBER_EXARGS_BUCKETS	17
#endif
int num_exargs_buckets = NUMBER_EXARGS_BUCKETS;

/*********************************************
 * required global variables
 *********************************************/

static String usage_summary = "";      /* usage string */
bool debugmode = false;     /* no signal handling, so we can get coredump */
bool opt_nocb  = false;     /* no cb. data is displayed if TRUE */
bool keyflag   = true;      /* show key values */
int alldone       = 0;         /* completion flag */
extern bool progrunning;
extern bool progparsing;
extern int     progerror;
bool traditional = true;    /* use traditional family rules */
bool showusage = false;     /* show usage */
bool showversion = false;   /* show version */
String  ext_codeset = 0;       /* default codeset from locale */
//int screen_width = 20; /* TODO */

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void print_usage(void);
static void load_usage(void);
static void main_db_notify(String db, bool opening);
static void parse_arg(const char * optarg, char ** optname, char **optval);
static void platform_init(void);

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
	bool ok=false;
	bool python_interactive = false;
	String dbrequested=NULL; /* database (path) requested */
	int alteration=0;
	List *exprogs=NULL;
	TABLE exargs=NULL;
	String progout=NULL;
	String configfile=0;
	String crashlog=NULL;
	int i=0;
	bool have_python_scripts = false;

	current_uiio = uiio_stdio;

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
	
	/* capture user's default codeset */
	ext_codeset = strsave(ll_langinfo());
	/* TODO: We can use this info for default conversions */

#if ENABLE_NLS
	/* setup gettext translation */
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
			print_version("llexec");
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
			logAllocations(true);
			break;
		case 'd':	/* debug = no signal catchers */
			debugmode = true;
			break;
		case 'k':	/* don't show key values */
			keyflag = false;
			break;
		case 'n':	/* use non-traditional family rules */
			traditional = false;
			break;
#if !defined(DEADENDS)		/* XXX not currently supported by DeadEnds XXX */
		case 't': /* show lots of trace statements for debugging */
			prog_trace = true;
			break;
#endif
		case 'x': /* execute program */
			if (!exprogs) {
				exprogs = create_list2(LISTDOFREE);
			}
			push_list(exprogs, strdup(optarg ? optarg : ""));
			break;
		case 'I': /* program arguments */
			{
				String optname=0, optval=0;
				parse_arg(optarg, &optname, &optval);
				if (optname && optval) {
					if (!exargs) {
						exargs = createStringTable(num_exargs_buckets);
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
		case 'C': /* specify config file */
			configfile = optarg;
			break;
		case 'P':	/* python interactive */
			python_interactive = true;
			break;
		case 'p':
#if HAVE_PYTHON
			llpy_register_script (optarg);
#endif
			have_python_scripts = true;
			break;
		case 'v': /* show version */
			showversion = true;
			goto usage;
			break;
		case 'h': /* show usage */
		case '?': /* show usage */
			showversion = true;
			showusage = true;
			goto usage;
			break;
		}
	}

prompt_for_db:

	/* catch any fault, so we can close database */
	if (!debugmode)
	{
		set_signals(sighand_cmdline);
	}
	/* developer wants to drive without seatbelt! */
	else
	{
		stdstring_hardfail();
		/* yydebug = 1; */
	}

	platform_init();
	set_displaykeys(keyflag);

	/* initialize options & misc. stuff */
	llgettext_set_default_localedir(LOCALEDIR);
	if (!init_lifelines_global(configfile, &msg, &main_db_notify)) {
		llwprintf("%s", msg);
		goto finish;
	}
	/* setup crashlog in case init_screen fails (eg, bad menu shortcuts) */
	crashlog = getlloptstr("CrashLog_llexec", NULL);
	if (!crashlog) { crashlog = "Crashlog_llexec.log"; }
	crash_setcrashlog(crashlog);
	/* give interpreter its turn at initialization */
	initializeInterpreter(currentDatabase);

	c = argc - optind;
	if (c > 1) {
		showusage = true;
		goto usage;
	}

	if (exargs) {
		set_cmd_options(exargs);
		release_table(exargs);
		exargs = 0;
	}
	/* Open database, prompting user if necessary */
	if (1) {
		String errmsg=0;
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
	/* does not use show module */
	/* does not use browse module */
	if (exprogs) {
		bool picklist = false;
		bool timing = false;
		interp_main(exprogs, progout, picklist, timing);
		destroy_list(exprogs);
	} else {
		/* TODO: prompt for report filename */
	}
	if (have_python_scripts) {
#if HAVE_PYTHON
		int status = llpy_execute_scripts (false);
		if (status < 0)
			ok = false;
		else
			ok = true;
#else
		fprintf (stderr, "Sorry, but Python support is not available in this version of Lifelines\n");
		ok=false;
#endif
	}
	if (python_interactive) {
#if HAVE_PYTHON
		int status = llpy_python_interactive ();
		if ((status == 0))
			ok=true;
		else
			ok=false;
#else
		fprintf (stderr, "Sorry, but Python support is not available in this version of Lifelines\n");
		ok=false;
#endif
	}
	/* does not use show module */
	/* does not use browse module */
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
	shutdown_ui(!ok);
	if (alldone == 2)
		goto prompt_for_db; /* changing databases */
	termlocale();
	strfree(&ext_codeset);

usage:
	/* Display Version and/or Command-Line Usage Help */
	if (showversion) { print_version("llexec"); }
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
			int namelen = ptr - optarg;
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
 * shutdown_ui -- (Placeholder, we don't need it)
 *=================================================*/
void
shutdown_ui (ATTRIBUTE_UNUSED bool pause)
{
}
/*==================================================
 * platform_init -- platform specific initialization
 *================================================*/
static void
platform_init (void)
{
	/* TODO: We could do wtitle just like llines, but its declaration needs
	to be moved somewhere more sensible for that (ie, not in curses.h!) */
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
	char * exename = "llexec";
	print_lines_usage(exename);
}
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

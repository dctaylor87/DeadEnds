/* deadends.c -- load a GEDCOM file and then

   run a DE script, or
   run a Python script, or
   invoke the interactive Python interpreter. */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdint.h>

#if defined(HAVE_GETOPT)
#if defined(HAVE_GETOPT_H)
#include <getopt.h>
#endif
#endif

#include "porting.h"
#include "standard.h"
#include "denls.h"
#include "path.h"
#include "hashtable.h"
#include "refnindex.h"
#include "gnode.h"
#include "database.h"
#include "file.h"
#include "import.h"

#include "llpy-externs.h"
#include "deadends.h"

/* XXX These paths are simplistic and *SHOULD* be changed.  They are
   here to give us some not totally unreasonable default.  They can be
   overridden by environment variables and finally by command line
   switches. XXX */

const char *DEADENDS_search_path = ".:/usr/share/deadends:/usr/share/lifelines";
const char *GEDCOM_search_path = ".";
const char *PYTHON_search_path = "."; /* XXX not currently used XXX */

Database *currentDatabase;

/* forward references */
static void print_usage (int status);
static void print_version (void);
static int parse_option (const char *option);

const char *optstring = "hI:vp:Px:";

const struct option longopts[] =
  {
    { "deadend-script",     required_argument, 0, 'x' },
    { "help",               no_argument,       0, 'h' },
    { "option",             required_argument, 0, 'I' },
    { "python-interactive", no_argument,       0, 'P' },
    { "python-script",      required_argument, 0, 'p' },
    { "version",            no_argument,       0, 'v' },
    { NULL,                 0,                 0, 0 }
  };

const char *ProgName;

int
main (int argc, char *argv[])
{
  int python_interactive = 0;
  int have_python_scripts = 0;
  int have_deadend_scripts = 0;
  int opt;
  int longindex = 0;
  char *cmd_line_db;
  //File *db_file;
  ErrorLog *error_log;
  const char *env;

  ProgName = argv[0];

#if HAVE_SETLOCALE
  /* initialize locales */
  setlocale(LC_ALL, "");
#endif

#if ENABLE_NLS
  /* setup gettext translation */
  bindtextdomain(PACKAGE, LOCALEDIR);
  textdomain(PACKAGE);
#endif

  env = getenv ("DEGEDCOM");
  if (env)
    GEDCOM_search_path = env;

  env = getenv ("DESCRIPTS");
  if (env)
    DEADENDS_search_path = env;

  env = getenv ("DEPYTHON");
  if (env)
    PYTHON_search_path = env;

  opterr = 0;			/* we prefer to print our own messages */
  while ((opt = getopt_long (argc, argv, optstring, longopts, &longindex)) != -1)
    {
      switch (opt)
	{
	case 'h':
	  print_usage (0);
	  break;

	case 'v':
	  print_version ();
	  break;
	  
	case 'I':
	  if (parse_option (optarg) < 0)
	    {
	      fprintf (stderr, "%s: unrecognized option: '%s'\n", ProgName, optarg);
	      print_usage(1);
	    }
	  break;

	case 'p':
	  llpy_register_script (optarg);
	  have_python_scripts = 1;
	  break;

	case 'P':
	  python_interactive = 1;
	  break;

	case 'x':
	  deadend_register_script (optarg);
	  have_deadend_scripts = 1;
	  break;

	case ':':		/* required argument is missing */
	  fprintf (stderr, "%s: option '%c' requires an argument\n",
		   ProgName, optopt);
	  print_usage (1);
	  break;

	case '?':		/* unrecognized option */
	  fprintf (stderr, "%s: option '%c' is unrecognized\n", ProgName, optopt);
	  print_usage (1);
	  break;

	default:
	  fprintf (stderr, "%s: unexpected return value %d from getopt_long\n",
		   ProgName, opt);
	  exit (1);
	}
    }
  if (! have_python_scripts && ! python_interactive && ! have_deadend_scripts)
    print_usage (1);		/* either -p or -P or -x is required */

  cmd_line_db = argv[optind];

  error_log = createErrorLog ();
#if 1
  CString resolved_file = resolveFile (cmd_line_db, GEDCOM_search_path);
  if (! resolved_file)
    {
      fprintf (stderr, "%s: resolveFile failed to find '%s'\n",
	       ProgName, cmd_line_db);
      exit (1);
    }

  currentDatabase = getDatabaseFromFile (resolved_file, 0, error_log);
#else
  File *db_file = openFile (cmd_line_db, "r");
  if (! db_file)
    {
      fprintf (stderr, "%s: fopenPath failed to open '%s': %s\n",
	       ProgName, cmd_line_db, strerror (errno));
      exit (1);
    }

  currentDatabase = importFromFileFP (db_file, cmd_line_db, error_log);
#endif
  if (! currentDatabase)
    {
      fprintf (stderr, "%s: import failed\n", ProgName);
      showErrorLog (error_log);
      exit (1);
    }

  if (have_deadend_scripts)
    {
      int status = deadend_execute_scripts (0, currentDatabase);
      if (status < 0)
	fprintf (stderr, "%s: DeadEnds script failed, status = %d\n",
		 ProgName, status);
    }

  /* so that output above and Python output is not intermixed */
  fflush (stdout);

  if (have_python_scripts)
    {
      int status = llpy_execute_scripts (0);
      if (status < 0)
	fprintf (stderr, "%s: Python script failed, status = %d\n",
		 ProgName, status);
    }
  if (python_interactive)
    {
      int status = llpy_python_interactive ();
      if (status != 0)
	fprintf (stderr, "%s: Python interactive returned status = %d\n",
		 ProgName, status);
    }
  if (python_interactive || have_python_scripts)
    llpy_python_terminate ();

  return (0);
}

static void
print_usage (int status)
{
  FILE *stream = ((status == 0) ? stdout : stderr);
  fprintf (stream, "usage: %s [options] <database>\n", ProgName);
  fprintf (stream, "options:\n\n");
  fprintf (stream, "--deadend-script | -x <script-name>\n");
  fprintf (stream, "\tRun a DeadEnd script\n");
  fprintf (stream, "--python-interactive | -P\n");
  fprintf (stream, "\tInvoke the interactive Python interpreter\n");
  fprintf (stream, "--python-script | -p <script-name>\n");
  fprintf (stream, "\tRun a Python script\n");
  fprintf (stream, "--help | -h\n");
  fprintf (stream, "\tPrint a help (usage) message and exit\n");
  fprintf (stream, "--version | -v\n");
  fprintf (stream, "\tPrint version information and exit\n");
  fprintf (stream, "--option | -I OPTION=VALUE\n");
  fprintf (stream, "\trecognized OPTIONS are: DEADENDS, GEDCOM, and PYTHON\n");
  fprintf (stream, "\tand specify search paths for DeadEnds scripts, GEDCOM\n");
  fprintf (stream, "\tfiles, and Python scripts, respectively\n");

  exit (status);
}

static void
print_version (void)
{
  fprintf (stdout, "%s\n", version);
  exit (0);
}

static int
parse_option (const char *option)
{
  switch (option[0])
    {
    case 'D':
      if (strncmp ("DEADENDS=", option, 9) == 0)
	{
	  DEADENDS_search_path = &option[9];
	  return (0);
	}
      return (-1);

    case 'G':
      if (strncmp ("GEDCOM=", option, 7) == 0)
	{
	  GEDCOM_search_path = &option[7];
	  return (0);
	}
      return (-1);

    case 'P':
      if (strncmp ("PYTHON=", option, 7) == 0)
	{
	  PYTHON_search_path = &option[7];
	  return (0);
	}
      return (-1);

    default:
      return (-1);
    }
}

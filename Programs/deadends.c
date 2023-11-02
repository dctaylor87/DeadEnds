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

#if defined(HAVE_GETOPT)
#if defined(HAVE_GETOPT_H)
#include <getopt.h>
#endif
#endif

#include "porting.h"
#include "standard.h"
#include "path.h"
#include "gnode.h"
#include "database.h"
#include "import.h"

#include "llpy-externs.h"
#include "deadends.h"

/* forward references */
static void print_usage (int status);
static void print_version (void);

const char *optstring = "hvp:Px:";

const struct option longopts[] =
  {
    { "deadend-script",     required_argument, 0, 'x' },
    { "help",               no_argument,       0, 'h' },
    { "version",            no_argument,       0, 'v' },
    { "python-interactive", no_argument,       0, 'P' },
    { "python-script",      required_argument, 0, 'p' },
    { NULL,                 0,                 0, 0 }
  };

const char *ProgName;
const char *searchpath = ".:../Gedfiles:$LLDATABASES:$HOME";

int
main (int argc, char *argv[])
{
  int python_interactive = 0;
  int have_python_scripts = 0;
  int have_deadend_scripts = 0;
  int opt;
  int longindex = 0;
  char *cmd_line_db;
  FILE *db_file;
  ErrorLog error_log;

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

  /* XXX insert code to open cmd_line_db XXX */

  db_file = fopenpath (cmd_line_db, "r", searchpath);
  if (! db_file)
    {
      fprintf (stderr, "%s: fopenpath failed to open '%s': %s\n",
	       ProgName, cmd_line_db, strerror (errno));
      exit (1);
    }

  theDatabase = simpleImportFromFile (db_file, &error_log);
  if (! theDatabase)
    {
      fprintf (stderr, "%s: import failed\n", ProgName);
      /* XXX figure out how to print 'error_log' XXX */
      exit (1);
    }
  if (have_deadend_scripts)
    {
      int status = deadend_execute_scripts (0);
      if (status < 0)
	fprintf (stderr, "%s: DeadEnds script failed, status = %d\n",
		 ProgName, status);
    }
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

  exit (status);
}

static void
print_version (void)
{
  fprintf (stdout, "%s\n", version);
  exit (0);
}

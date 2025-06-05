/* testpython.c -- load a GEDCOM file and then either invoke a Python
   script or invoke the interactive Python interpreter. */

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
#include <locale.h>		/* setlocale, LC_ALL */
#include <libintl.h>		/* bindtextdomain */

#if defined(HAVE_GETOPT)
#if defined(HAVE_GETOPT_H)
#include <getopt.h>
#endif
#endif

#include "porting.h"
#include "standard.h"
#include "path.h"
#include "errors.h"
#include "file.h"
#include "gedcom.h"
#include "gnode.h"
#include "database.h"
#include "import.h"

#include "llpy-externs.h"

/* forward references */
static void print_usage (int status);
static void print_version (void);

const char *optstring = "hvp:P";

const struct option longopts[] =
  {
    { "help",               no_argument,       0, 'h' },
    { "version",            no_argument,       0, 'v' },
    { "python-interactive", no_argument,       0, 'P' },
    { "python-script",      required_argument, 0, 'p' },
    { NULL,                 0,                 0, 0 }
  };

const char *ProgName;
const char *searchpath = ".:../Gedfiles:$DEDATABASES:$HOME";

Database *currentDatabase = 0;

int
main (int argc, char *argv[])
{
  int python_interactive = 0;
  int have_python_scripts = 0;
  int opt;
  int longindex = 0;
  CString cmd_line_db;
  ErrorLog *error_log;

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
  if (! have_python_scripts && ! python_interactive)
    print_usage (1);		/* either -p or -P is required */

  cmd_line_db = argv[optind];

  if (! cmd_line_db)
    {
      fprintf (stderr, "%s: no database specified on command line\n", ProgName);
      print_usage (1);
    }
  /* XXX insert code to open cmd_line_db XXX */

  cmd_line_db = resolveFile (cmd_line_db, searchpath);
  if (! cmd_line_db)
    {
      fprintf (stderr, "%s: database not found along search path: '%s'\n",
	       ProgName, searchpath);
      exit (1);
    }

  //currentDatabase = gedcomFileToDatabase (cmd_line_db, &error_log);
  error_log = createErrorLog ();
  currentDatabase = getDatabaseFromFile (cmd_line_db, 0, error_log);

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
  fprintf (((status == 0) ? stdout : stderr),
	   "%s: sorry, but usage is not yet implemented\n", ProgName);
  exit (status);
}

static void
print_version (void)
{
  fprintf (stdout, "%s\n", version);
  exit (0);
}

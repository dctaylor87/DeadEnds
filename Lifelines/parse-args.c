/* "The MIT license"

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

/* parse-args.c -- Code for parsing args for both llines and llexec.
   Some of the cases in the switch statement only apply to one of the
   programs.  This is handled by having different getopt strings.  */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#ifdef HAVE_GETOPT
#ifdef HAVE_GETOPT_H
#include <getopt.h>
#endif /* HAVE_GETOPT_H */
#endif /* HAVE_GETOPT */

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "standard.h"

#include "stringtable.h"
#include "gnode.h"
#include "recordindex.h"
#include "database.h"
#include "list.h"

#include "readwrite.h"
#include "de-strings.h"
#include "options.h"
#include "parse-args.h"

#if defined(HAVE_PYTHON)
#include "llpy-externs.h"
#endif

#if !defined(NUMBER_EXARGS_BUCKETS)
#define NUMBER_EXARGS_BUCKETS	17
#endif
int num_exargs_buckets = NUMBER_EXARGS_BUCKETS;

List *exprogs = NULL;		/* DE script programs to be run, if any */
StringTable *exargs = NULL;	/* key=value pairs, but currently not very usable */
String progout = NULL;		/* were to write script output */

bool python_interactive = false;
bool have_python_scripts = false;

String configfile = NULL;

bool graphical=true;		/* only relevant for llines, not llexec */
int winx = 0;			/* only relevant for llines */
int winy = 0;			/* only relevant for llines */

static void parse_arg(const char * optarg, char ** optname, char **optval);
static void showUsage(int exitCode);

void
parseArguments (int argc, char *argv[], CString optString)
{
  int c;

  /* handle conventional arguments --version and --help */
  /* needed for help2man to synthesize manual pages */
  for (int i=1; i<argc; ++i) {
    if (!strcmp(argv[i], "--version")
	|| !strcmp(argv[i], "-v")) {
      print_version(ProgName);
      exit (0);
    }
    if (!strcmp(argv[i], "--help")
	|| !strcmp(argv[i], "-h")
	|| !strcmp(argv[i], "-?")) {
      print_usage();
      exit (0);
    }
  }

  /* Parse Command-Line Arguments */
  opterr = 0;	/* turn off getopt's error message */
  while ((c = getopt(argc, argv, optString)) != -1) {
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
    case 'u': /* specify screen dimensions */
      sscanf(optarg, SCN_INT "," SCN_INT, &winx, &winy);
      break;
    case 'x': /* execute program */
      if (!exprogs) {
	exprogs = createList (NULL, NULL, free, false);
      }
      pushList(exprogs, strdup(optarg ? optarg : ""));
      break;
    case 'I': /* program arguments */
      {
	String optname=0, optval=0;
	parse_arg(optarg, &optname, &optval);
	if (optname && optval) {
	  if (!exargs) {
	    exargs = createStringTable(num_exargs_buckets);
	  }
	  addToStringTable(exargs, optname, optval);
	}
	strfree(&optname);
	strfree(&optval);
      }
      break;
    case 'o': /* output directory */
      progout = optarg;
      break;
    case 'z': /* nongraphical box */
      graphical = false;
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
      showVersion();
      exit (0);			/* not reached */
    case 'h': /* show usage */
      showUsage(0);
      exit (0);			/* not reached */
    case ':':
      fprintf (stderr, "%s: option '%c' requires an option\n", ProgName, optopt);
      showUsage(1);
      exit (1);
    case '?': /* show usage */
      fprintf (stderr, "%s: option '%c' is unrecognized\n", ProgName, optopt);
      showUsage(1);
      exit (1);			/* not reached */
      break;
    }
  }

}

/* parse_arg -- Break argument into name & value pairs
   e.g., parse_arg("main_indi=I3", &a, &b)
   yields a="main_indi" and b="I3"
   (a & b are newly allocated from heap)  */

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
			namebuff = (char *)stdalloc(namelen+1);
			destrncpy(namebuff, optarg, namelen+1, 0);
			*optname = namebuff;
			valbuff = strdup(ptr+1);
			*optval = valbuff;
			return;

		}
	}
}

static void
showUsage(int exitCode)
{
  fprintf ((exitCode ? stderr : stdout), "usage: %s %s\n", ProgName, usageSummary);
  exit (exitCode);
}

/*=============================================================
 * lines_usage.c -- Usage display for Lifelines
 *  for main executable and for llexec
 *  generates --help output, so also used by help2man
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "denls.h"

#include "rfmt.h"
#include "recordindex.h"
#include "sequence.h"
#include "ask.h"
#include "refnindex.h"

#include "llinesi.h"
#include "messages.h"

/*===============================================
 * print_lines_usage -- display program help/usage
 *  displays to stdout
 *  exename: [IN]  "Lines" or "llines" or "llexec"
 *=============================================*/
void
print_lines_usage (CString exename)
{
	if (0 == strcmp(exename, "llexec")) {
		printf(_("%s invokes the DeadEnds report execution program without GUI."), exename);
	} else {
		printf(_("%s invokes the DeadEnds GUI."), exename);
	}
	printf("\n\n");
	printf(_("DeadEnds is a program to manipulate genealogical information\n"
		 "in lineage-linked GEDCOM format.  It has a curses interface and two\n"
		 "built-in interpreters, one for its own genealogical report language\n"
		 "and one for Python.\n"
		 "\n"
		 "It reads GEDCOM files and keeps its database entirely in memory.\n"
		 "Records are edited directly in GEDCOM (with an editor of the user's\n"
		 "choice).  Records store information about people, families, sources,\n"
		 "events, and other types of data (such as notes).  DeadEnds includes\n"
		 "both a custom report language (and comes with a collection of already\n"
		 "written reports) and an embedded Python interpreter.")
	       );
	printf("\n\n");
	printf(_("Usage: %s [OPTIONS] [database]"), exename);
	printf("\n\n");
	printf(_("Options:"));
	printf("\n\t");
	printf(_("-C[FILE]"));
	printf("\n\t\t");
	printf(_("Specify configuration file location"));
	printf("\n\t-F\n\t\t");
	printf(_("Finnish option (if compiled to allow Finnish option)"));
	printf("\n\t");
	printf(_("-I[KEY=VALUE]"));
	printf("\n\t\t");
	printf(_("Specify a user property (e.g. -IDEEDITOR=gvim)"));
	printf("\n\t-a\n\t\t");
	printf(_("log dynamic memory operation (for debugging)"));
	printf("\n\t");
	printf("\n\t-d\n\t\t");
	printf(_("debug mode (disable signal trapping)"));
	printf("\n\t-f\n\t\t");
	printf(_("force open (for recovery from errors or power failure)"));
	printf("\n\t-k\n\t\t");
	printf(_("always show keys (even if REFN available)"));
	printf("\n\t-ln\n\t\t");
	printf(_("unlock a database (for correct access on read-write media)"));
	printf("\n\t-ly\n\t\t");
	printf(_("lock a database (for use on read-only media)"));
	printf("\n\t-n\n\t\t");
	printf(_("do not use traditional family rules"));
	printf("\n\t-o[FILE]\n\t\t");
	printf(_("Specify program output filename (eg, -o/tmp/mytests)"));
	printf("\n\t-t\n\t\t");
	printf(_("trace function calls in report programs (for debugging)"));
	printf("\n\t");
	printf(_("-u[HEIGHT,WIDTH]"));
	printf("\n\t\t");
	printf(_("specify window size (eg, -u120,34 specifies 120 columns\n"
		"\t\tby 34 rows)"));
	printf("\n\t--help\n\t\t");
	printf(_("display this help and exit"));
	printf("\n\t");
	printf(_("-x[REPORT]"));
	printf("\n\t\t");
	printf(_("execute a single DeadEnds report program directly"));
	printf("\n\t-z\n\t\t");
	printf(_("Use normal ASCII characters for drawing lines, instead of\n"
		"\t\tspecial VT100 terminal characters"));
	printf("\n\t--version\n\t\t");
	printf(_("output version information and exit"));
	printf("\n\n");
	printf(_("Examples:"));
	printf("\n\t");
	if (0 == strcmp(exename, "llexec")) {
	    printf(_("%s myfamily -x eol"), exename);
	    printf("\n\t\t");
	    printf(_("Open the database 'myfamily' with DeadEnds"));
	    printf("\n\t\t");
	    printf(_("and run the eol.ll report"));
	    printf("\n\n");
	} else {
	    printf(_("%s myfamily"), exename);
	    printf("\n\t\t");
	    printf(_("Open database 'myfamily' with DeadEnds"));
	    printf("\n\n");
	}
	printf(_("REPORTING BUGS"));
	printf("\n\t");
	printf("%s", _(qSgen_bugreport));
	printf("\n\n");
	printf(_("COPYRIGHT"));
	printf("\n\t");
        printf("%s", _(qSgen_copyright));
	printf("\n\n");
	printf(_("LICENSE"));
	printf("\n\t");
        printf("%s", _(qSgen_license));
	printf("\n\n");
	printf(_("SEE ALSO"));
	printf("\n\tllines(1), llexec(1)");
	printf("\n");
}

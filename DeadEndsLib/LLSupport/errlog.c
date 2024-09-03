/* This file is the merger or the LifeLines files stdlib/error.c, stdlib/errlog.c,
   with modifications.  Portions copyright 1991-1999 by Thomas T. Wetmore, IV.
   Portions copyright 2005 by Perry Rapp.  Portions copyright by others.

   Consult Git on GitHub and CVS on sourceforge for the blow-by-blow.  */

/* Permission is hereby granted, free of charge, to any person
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
   SOFTWARE.  */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>
#include <stdarg.h>
#include <time.h>

#include "standard.h"
#include "denls.h"
#include "sys_inc.h"

//#include "liflines.h"
#include "gnode.h"
#include "database.h"
#include "feedback.h"
#include "readwrite.h"
#include "errlog.h"
#include "de-strings.h"
#include "codesets.h"		/* uu8 */

/* local variables */

static char f_crashfile[MAXPATHLEN]="";
static char f_currentdb[MAXPATHLEN]="";

/* local functions */
static void vcrashlog(int newline, const char * fmt, va_list args);
static void get_current_lldate (char *creation);

#if !defined(DEADENDS)
/*
 2002/10/05
 These routines do not depend on curses (llwprintf can be implemented w/o curses)
 and so could move out of the liflines subdir. llexec is using this file.
*/

/* __fatal -- Fatal error routine.  Handles null or empty details input.  */
void
__fatal (String file, int line, CString details)
{
  /* avoid reentrancy */
  static bool failing=false;
  if (!failing)
    {
      failing=true;

      /* send to error log if one is specified */
      errlog_out(_("Fatal Error"), details, file, line);

      /* send to screen */
      llwprintf("%s\n", _("FATAL ERROR"));
      if (details && details[0]) {
	llwprintf("  %s\n", details);
      }
      llwprintf(_("  in file <%s> at line %d\n"), file, line);

      /* offer crash dump before closing database */
      ll_optional_abort(_("ASSERT failure"));
      close_lifelines();
      shutdown_ui(true); /* pause */

      failing=false;
    }
  exit(1);
}
#endif

/* vcrashlog -- Send crash info to crash log and screen. */
static void
vcrashlog (int newline, const char * fmt, va_list args)
{
  char buffer[2048];
  vsnprintf(buffer, sizeof(buffer), fmt, args);
  buffer[sizeof(buffer)-1] = 0;
  if (newline) {
    /* ensure room at end to append \n */
    buffer[sizeof(buffer)-2] = 0;
    strcat(buffer, "\n");
  }

  /* send to error log if one is specified */
  errlog_out(NULL, buffer, NULL, -1);

  /* send to screen */
  llwprintf("%s", buffer);
}

/* crashlog -- Send crash info to crash log and screen */
void
crashlog (String fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vcrashlog(0, fmt, args);
  va_end(args);
}

/* crashlogn -- Send crash info to crash log and screen
   add carriage return to end line. */
void
crashlogn (String fmt, ...)
{
  va_list args;
  va_start(args, fmt);
  vcrashlog(1, fmt, args);
  va_end(args);
}

/* errlog_out -- Send message to log file (if one exists). */
void
errlog_out (CString title, CString msg, CString file, int line)
{
  /* avoid reentrancy */
  static bool failing=false;
  if (failing) return;
  failing=true;

  /* send to error log if one is specified */
  if (f_crashfile[0]) {
    FILE * fp = fopen(f_crashfile, DEAPPENDTEXT);
    if (fp) {
      char creation[DATE_STR_LEN];
      get_current_lldate(creation);
      if (title) {
	fprintf(fp, "\n%s: %s\n", title, creation);
	if (msg && msg[0]) {
	  fprintf(fp, "    %s\n", msg);
	}
      } else {
	if (msg && msg[0]) {
	  fprintf(fp, "%s: %s\n", creation, msg);
	} else {
	  fprintf(fp, "%s\n", creation);
	}
      }
      if (line>=1 && file && file[0])
	fprintf(fp, _("    in file <%s> at line %d\n"), file, line);
      if (f_currentdb[0])
	fprintf(fp, "    %s: %s\n", _("Current database"), f_currentdb);
      fclose(fp);
    }
  }
  failing=false;
}

/* crash_setcrashlog -- specify where to log alloc messages.  */
void
crash_setcrashlog (String crashlog)
{
  if (!crashlog)
    crashlog = "";
  destrncpy(f_crashfile, crashlog, sizeof(f_crashfile), uu8);
}

/* crash_setdb -- record current database in case of a crash
   Created: 2002/06/16, Perry Rapp.  */
void
crash_setdb (String dbname)
{
  if (!dbname)
    dbname = "";
  destrncpy(f_currentdb, dbname, sizeof(f_currentdb), 0);
}

/* get_current_lldate -- fill in ISO style string for current time.  */
static void
get_current_lldate (char *creation)
{
  struct tm *pt;
  time_t curtime;
  curtime = time(NULL);
  pt = gmtime(&curtime);
  snprintf(creation, DATE_STR_LEN, "%04d-%02d-%02d-%02d:%02d:%02dZ", 
	   pt->tm_year+1900, pt->tm_mon+1, pt->tm_mday,
	   pt->tm_hour, pt->tm_min, pt->tm_sec);
}

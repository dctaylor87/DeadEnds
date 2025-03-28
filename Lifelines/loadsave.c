/* 
   Copyright (c) 2002 Perry Rapp

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
 * loadsave.c -- curses user interface for loading & saving GEDCOM
 * NB: Part of curses GUI version
 * Copyright(c) 2002 by Perry Rapp; all rights reserved
 *   Created: 2002/06 by Perry Rapp
 *==============================================================*/


#include <time.h>


#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "denls.h"

#include "gnode.h"
#include "database.h"
#include "locales.h"
#include "zstr.h"
#include "translat.h"
#include "xlat.h"
#include "readwrite.h"
#include "recordindex.h"
#include "rfmt.h"
#include "sequence.h"
#include "writenode.h"
#include "ask.h"
#include "feedback.h"
#include "llinesi.h"
#include "liflines.h"
#include "messages.h"
#include "screen.h"
#include "lloptions.h"
#include "de-strings.h"
#include "import.h"
#include "codesets.h"
#include "version.h"
#include "editing.h"
#include "dateprint.h"
#include "loadsave.h"

/* local variables */

static XLAT xlat_gedout; /* TODO: could do away with this via param to traverse */

static char *mabbv[] =
  {
    "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC",
  };

/* load_gedcom -- have user select gedcom file & import it.  */

/* LifeLines first validates the file and then reads the file
   a second time putting it into the database.  But, DeadEnds
   is different -- it reads the file once, creates the
   database, validates it, and then if there are errors,
   throws away the database.

   Since we are potentially adding to an existing database, we
   do it slighly differently.  We create a new database, we
   validate it, if errors, we discard.  If no errors, we merge
   into the existing database and discard the new database.

   There are two arguments -- picklist and database.

   For the old (LifeLines) behavior, database is the database
   to merge into (likely currentDatabase).  If database is
   NULL, then this will be a way to create a new database from
   the curses UI.  */

void
load_gedcom (bool picklist, Database *database)
{
	String srcdir=NULL;
	String fullpath=0;
	time_t begin = time(NULL);
	time_t beginui = get_uitime();

	srcdir = getdeoptstr("InputPath", ".");
	if (! ask_for_gedcom(DEREADTEXT, _(qSwhatgedc), 0, &fullpath, srcdir, ".ged", picklist)) {
		strfree(&fullpath);
		return;
	}

	ErrorLog *errorLog = createErrorLog ();
	Database *newDB = selectAndOpenDatabase (&fullpath, srcdir, database, errorLog);

	if (newDB)
	  {
	    /* if database is non-NULL, then the file was merged into
	       database, newDB == database, and the database list does
	       not need to be updated.  */
	    if (! database)
	      {
		/* otherwise newDB is truly new.  Add it to the
		   database list.  */
		insertInDatabaseList (newDB);
	      }
	  }
	else
	  {
	    /* XXX selectAndOpenDatabase failed, show the error log
	       and offer to put it into a file.  XXX */
	  }
	deleteErrorLog (errorLog);

	int duration = time(NULL) - begin;
	int uitime = get_uitime() - beginui;
	ZSTR zt1=approx_time(duration-uitime), zt2=approx_time(uitime);
	/* TRANSLATORS: how long Import ran, and how much of that was UI delay */
	ZSTR zout = zs_newf(_("Import time %s (ui %s)\n")
			    , zs_str(zt1), zs_str(zt2));
	wfield(8,0, zs_str(zout));
	zs_free(&zt1);
	zs_free(&zt2);
	zs_free(&zout);

	/* position cursor further down stdout so check_stdout 
	doesn't overwrite our messages from above */
	wpos(15,0);
}

/*================================
 * save_gedcom -- save gedcom file
 *==============================*/

bool
save_gedcom (Database *database)
{
	FILE *fp=NULL;
	//struct tag_export_feedback efeed;
	String srcdir=NULL, fname=0, fullpath=0;

	if (! database)
		database = currentDatabase;

	srcdir = getdeoptstr("DEARCHIVES", ".");
	fp = ask_for_output_file(DEWRITETEXT, _(qSoutarc), &fname, &fullpath, srcdir, ".ged");
	if (!fp) {
		strfree(&fname);
		msg_error("%s", _("The database was not saved."));
		return false; 
	}

	llwprintf(_("Saving database `%s' in file `%s'."), database->name, fullpath);

	write_header (fp);

	write_body (fp, database);

	write_trailer (fp);
	fclose(fp);

	wpos(7,0);
	msg_info(_(qSoutfin), database->name, fname);
	strfree(&fname);
	
	return true;
}

/* XXX needs to be adapted for DE.  We should support GEDCOM 5.5.1 or
   7.0.  And we should consider having the get..opt.. functions take a
   Database* as an argument -- that is, I can imagine having Database
   specific option values.  XXX */

void
write_header (FILE *fp)
{
  char dat[30]="", tim[20]="";
  struct tm *pt=0;
  time_t curtime;
  String str=0;
  xlat_gedout = transl_get_predefined_xlat(MINGD); /* internal to GEDCOM */

  /* reportedly needed on Windwos; neither Linux nor MacOS needs it.  */
  prefix_file_for_gedcom(fp);

  curtime = time(NULL);
  pt = localtime(&curtime);
  snprintf(dat, sizeof(dat), "%d %s %d", pt->tm_mday, mabbv[pt->tm_mon],
	   1900 + pt->tm_year);
  snprintf(tim, sizeof(tim), "%d:%.2d", pt->tm_hour, pt->tm_min);

  fprintf(fp, "0 HEAD\n1 SOUR LIFELINES %s\n1 DEST ANY\n",
	  get_deadends_version(80));

  /* header date & time */
  fprintf(fp, "1 DATE %s\n2 TIME %s\n", dat, tim);

  /* header submitter entry */
  str = getdeoptstr("HDR_SUBM", "1 SUBM");
  fprintf(fp, "%s\n", str);

  /* header gedcom version info */
  str = getdeoptstr("HDR_GEDC", "1 GEDC\n2 VERS 5.5\n2 FORM LINEAGE-LINKED");
  fprintf(fp, "%s\n", str);

  /* header character set info -- should be outcharset; that is what
     is being used */

  str = getdeoptstr("HDR_CHAR", 0);
  if (str) {
    fprintf(fp, "%s\n", str);
  } else {
    /* xlat_gedout is the actual conversion used, so
       we should use the name of its output */
    CString outcharset = xl_get_dest_codeset(xlat_gedout);
    fprintf(fp, "1 CHAR %s\n", outcharset);
  }
  /* finished header */
}

void
write_body (FILE *fp, Database *database)
{
  FORHASHTABLE (database->recordIndex, element)
    writeGNodeRecord (fp, (GNode *) element, false);
  ENDHASHTABLE
}

void
write_trailer (FILE *fp)
{
  fprintf (fp, "0 TRLR\n");
}

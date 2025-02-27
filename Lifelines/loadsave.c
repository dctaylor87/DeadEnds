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

/*================================
 * load_gedcom -- have user select gedcom file & import it
 *==============================*/

void
load_gedcom (bool picklist, Databnase *database)
{
#if !defined(DEADENDS)
	FILE *fp=NULL;
	ImportFeedback ifeed;
#endif
	String srcdir=NULL;
	String fullpath=0;
	time_t begin = time(NULL);
	time_t beginui = get_uitime();

	srcdir = getdeoptstr("InputPath", ".");
#if defined(DEADENDS)
	if (! ask_for_gedcom(DEREADTEXT, _(qSwhatgedc), 0, &fullpath, srcdir, ".ged", picklist)) {
		strfree(&fullpath);
		return;
	}
#else
	if (!ask_for_gedcom(DEREADTEXT, _(qSwhatgedc), 0, &fullpath, srcdir, ".ged", picklist)
		|| !(fp = fopen(fullpath, DEREADBINARY))) {
		strfree(&fullpath);
		return;
	}
#endif
	/*
	Note: we read the file in binary mode, so ftell & fseek will work correctly.
	Microsoft's ftell/fseek do not work correctly if the file has simple unix (\n)
	line terminations! -- Perry, 2003-02-11
	*/

#if defined(DEADENDS)
	/* LifeLines first validates the file and then reads the file
	   a second time putting it into the database.  But, DeadEnds
	   is different -- it reads the file once, creates the
	   database, validates it, and then if there are errors,
	   throws away the database.

	   Since we are potentially adding to an existing database, we
	   do it slighly differently.  We create a new database, we
	   validate it, if errors, we discard.  If no errors, we merge
	   into the existing database and discard the new database.

	   Currently, there is one argument -- picklist, later there
	   will be two arguments -- picklist and database.  For the
	   old behavior, database will be the database to merge into
	   (likely currentDatabase).  If database is NULL, then this
	   will be a way to create a new database from the curses
	   UI.  */
	errorLog = createErrorLog ();
	Database *newDB = selectAndOpenDatabase (&fullpath, srcdir, database, errorLog);
	if (newDB)
	  {
	    /* XXX delete error log XXX */
	  }
#else
	memset(&ifeed, 0, sizeof(ifeed));
	ifeed.if_validating_fnc = import_validating;
	ifeed.if_validated_rec_fnc = import_validated_rec;
	ifeed.if_beginning_import_fnc = import_beginning_import;
	ifeed.if_error_invalid_fnc = import_error_invalid;
	ifeed.if_error_readonly_fnc = import_readonly;
	ifeed.if_adding_unused_keys_fnc = import_adding_unused_keys;
	ifeed.if_added_rec_fnc = import_added_rec;
	ifeed.if_validation_error_fnc = import_validation_error;
	ifeed.if_validation_warning_fnc =  import_validation_warning;

	import_from_gedcom_file(&ifeed, fp);
	
	fclose(fp);
	strfree(&fullpath);
#endif

	if (1) {
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
	}

	/* position cursor further down stdout so check_stdout 
	doesn't overwrite our messages from above */
	wpos(15,0);
}

#if 0				/* not yet */
/*================================
 * save_gedcom -- save gedcom file
 *==============================*/

bool
save_gedcom (Database *database)
{
	FILE *fp=NULL;
	struct tag_export_feedback efeed;
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
	prefix_file_for_gedcom(fp);

	memset(&efeed, 0, sizeof(efeed));
	efeed.added_rec_fnc = export_saved_rec;

	llwprintf(_("Saving database `%s' in file `%s'."), readpath_file, fullpath);

	/* Display 0 counts */
	clear_rec_counts(0);

	archive_in_file(&efeed, database, fp);
	fclose(fp);

	wpos(7,0);
	msg_info(_(qSoutfin), readpath_file, fname);
	strfree(&fname);
	
	return true;
}
#endif

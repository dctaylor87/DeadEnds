/* 
   Copyright (c) 1991-2005 Thomas T. Wetmore IV

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
 * selectdb.c -- Code handling user choice of database
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
#include "sys_inc.h"
#include "options.h"

#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "rfmt.h"
#include "refnindex.h"
#include "sequence.h"
#include "uiprompts.h"
#include "feedback.h"
#include "llinesi.h"
#include "errors.h"
#include "liflines.h"
#include "messages.h"
#include "codesets.h"
#include "screen.h"
#include "de-strings.h"
#include "ui.h"
#include "locales.h"
#include "hashtable.h"
#include "lloptions.h"
#include "path.h"

/*********************************************
 * local function prototypes
 *********************************************/

#if !defined(DEADENDS)
/* alphabetical */
static bool is_unadorned_directory(String path);
static void show_open_error(int dberr);
#endif

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==================================================
 * select_database -- open database (prompting if appropriate)
 * if fail, return FALSE, and possibly a message to display
 *  perrmsg - [OUT]  translated error message
 *================================================*/
bool
select_database (String * dbrequested, int alteration, String * perrmsg)
{
	String dbdir = getdeoptstr("DEDATABASES", ".");
	String dbused = NULL;
	ASSERT(dbrequested);
	ASSERT(*dbrequested);

	/* Get Database Name (Prompt or Command-Line) */
	if ((*dbrequested)[0] == '\0') {
		char dbname[MAXPATHLEN];
		/* ask_for_db_filename returns static buffer, we save it below */
		if (!ask_for_db_filename(_(qSidldir), _(qSidldrp), dbdir, dbname, sizeof(dbname))
			|| !dbname[0]) {
			*perrmsg = _(qSiddbse);
			return false;
		}
		strupdate(dbrequested, dbname);
		if (eqstr(*dbrequested, "?")) {
			int n=0;
			List *dblist=0, *dbdesclist=0;
			strfree(dbrequested);
			if ((n=get_dblist(dbdir, &dblist, &dbdesclist)) > 0) {
				int i;
				i = choose_from_list(
					_("Choose database to open")
					, dbdesclist);
				if (i >= 0) {
#if defined(DEADENDS)
					*dbrequested = strsave(getFromList(dblist, i+1));
#else
					*dbrequested = strsave(get_list_element(dblist, i+1, NULL));
#endif
				}
				release_dblist(dblist);
				release_dblist(dbdesclist);
			} else {
				*perrmsg = _("No databases found in database path");
				return false;
			}
			if (!*dbrequested) {
				*perrmsg = _(qSiddbse);
				return false;
			}
		}
	}

	/* search for database */
	/* search for file in lifelines path */
	dbused = filepath(*dbrequested, "r", dbdir, NULL, uu8);
	/* filepath returns alloc'd string */
	if (!dbused) dbused = strsave(*dbrequested);

	if (!open_or_create_database(alteration, &dbused)) {
		strfree(&dbused);
		return false;
	}

	stdfree(dbused);
	return true;
}

#if !defined(DEADENDS)
/*==================================================
 * open_or_create_database -- open database, prompt for
 *  creating new one if it doesn't exist
 * if fails, displays error (show_open_error) and returns 
 *  FALSE
 *  alteration:   [IN]  flags for locking, forcing open...
 *  dbused:       [I/O] actual database path (may be relative)
 * If this routine creates new database, it will alter dbused
 * Created: 2001/04/29, Perry Rapp
 *================================================*/
bool
open_or_create_database (int alteration, String *dbused)
{
	int lldberrnum=0;
	char dbdir[MAXPATHLEN] = "";
        char newmsg[MAXPATHLEN+100] = "";

	/* Open Database */
	if (open_database(alteration, *dbused, &lldberrnum))
		return true;

	/* filter out real errors */
	if (lldberrnum != BTERR_NODB && lldberrnum != BTERR_NOKEY)
	{
		show_open_error(lldberrnum);
		return false;
	}

	/*
	error was only that db doesn't exist, so lets try
	making a new one 
	*/
	/* First construct directory to use */
	if (is_unadorned_directory(*dbused)) {
		/* no dir specified, so use first from LLDATABASES */
		String dbpath = getdeoptstr("DEDATABASES", ".");
		CString newdbdir = get_first_path_entry(dbpath);
		if (newdbdir) {
			/* newdbdir is static from get_first_path_entry */
			newdbdir = strdup(newdbdir);
			pathConcat(newdbdir, *dbused, uu8, dbdir, sizeof(dbdir));
			stdfree((String)newdbdir);
		} else {
			destrncpy(dbdir, *dbused, sizeof(dbdir), uu8);
		}
	} else {
		destrncpy(dbdir, *dbused, sizeof(dbdir), uu8);
	}
	expand_special_fname_chars(dbdir, sizeof(dbdir), uu8);
	strupdate(dbused, dbdir);

	/* Is user willing to make a new db ? */
        snprintf(newmsg,sizeof(newmsg),qScrdbse,*dbused);
	if (!ask_yes_or_no_msg(_(qSnodbse), newmsg))
		return false;

	/* try to make a new db */
	if (create_database(*dbused, &lldberrnum))
		return true;

	show_open_error(lldberrnum);
	return false;
}
/*===================================================
 * show_open_error -- Display database opening error
 *=================================================*/
static void
show_open_error (int dberr)
{
	char buffer[256];
	describe_dberror(dberr, buffer, ARRAYSIZE(buffer));
	llwprintf("%s", buffer);
	llwprintf("\n");
	sleep(5);
}
/*==================================================
 * is_unadorned_directory -- is it a bare directory name,
 *  with no subdirectories ?
 * Created: 2001/01/24, Perry Rapp
 *================================================*/
static bool
is_unadorned_directory (String path)
{
	for ( ; *path; path++) {
		if (is_dir_sep(*path))
			return false;
	}
	return true;
}
#endif	/* !DEADENDS */

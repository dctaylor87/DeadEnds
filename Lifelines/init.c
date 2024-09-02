/* 
   Copyright (c) 1991-1999 Thomas T. Wetmore IV

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
 * init.c -- Initialize LifeLines data structures
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 29 Aug 93
 *   3.0.0 - 05 Oct 94    3.0.2 - 09 Nov 94
 *   3.0.3 - 21 Sep 95
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

#include "hashtable.h"
#include "stringtable.h"
#include "gnode.h"
#include "refnindex.h"
#include "database.h"
#include "codesets.h"
#include "llgettext.h"
#include "zstr.h"
#include "list.h"
#include "translat.h"
#include "xlat.h"
#include "sequence.h"
#include "browse.h"
#include "locales.h"
#include "de-strings.h"
#include "charprops.h"
#include "options.h"
#include "icvt.h"
#include "init.h"
#include "lloptions.h"

/* local defines -- overrides allowed, should be prime  */

#if !defined(NUMBER_OPTION_BUCKETS)
#define NUMBER_OPTION_BUCKETS	17
#endif
#if !defined(NUMBER_TAG_BUCKETS)
#define NUMBER_TAG_BUCKETS	53
#endif
#if !defined(NUMBER_PLACABBV_BUCKETS)
#define NUMBER_PLACABBV_BUCKETS	23
#endif

/*********************************************
 * global/exported variables
 *********************************************/

TABLE tagtable=NULL;		/* table for tag strings */
TABLE placabbvs=NULL;	/* table for place abbrevs */
String editstr=NULL; /* edit command to run to edit (has editfile inside of it) */
String editfile=NULL; /* file used for editing, name obtained via mktemp */
String readpath = NULL;		/* path to database */
String readpath_file = NULL;	/* final component of path to database */

/*********************************************
 * external/imported variables
 *********************************************/

extern String illegal_char;

/*********************************************
 * local function prototypes
 *********************************************/

static void check_installation_path(void);
static bool load_configs(String configfile, String * pmsg);
static void post_codesets_hook(void);
static void pre_codesets_hook(void);
static void update_db_options(void);

/*********************************************
 * local variables
 *********************************************/

static bool suppress_reload=false;
static char global_conf_path[MAXPATHLEN]="";

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=================================
 * init_lifelines_global -- Initialization options & misc. stuff
 *  This is called before first (or later) database opened
 *===============================*/
bool
init_lifelines_global (String configfile, String * pmsg, void (*notify)(String db, bool opening))
{
	String e;
	String dirvars[] = { "DEPROGRAMS", "DEREPORTS", "DEARCHIVES"
		, "DEDATABASES", };
	int i;

	check_installation_path();

	/* request notification when options change */
	register_notify(&update_useropts);
	suppress_reload = true;

	dbnotify_set(notify);

	if (!load_configs(configfile, pmsg)) {
		suppress_reload = false;
		update_useropts(NULL);
		return false;
	}

	pre_codesets_hook(); /* For MS-Windows user config of console codepages */

	/* now that codeset variables are set from config file, lets initialize codesets */
	/* although int_codeset can't be determined yet, we need GUI codeset for gettext */
	init_codesets();

	post_codesets_hook(); /* For Windows, link dynamically to gettext & iconv if available */


	/* until we have an internal codeset (which is until we open a database)
	we want output in display codeset */
	llgettext_init(PACKAGE, gui_codeset_out);

	/* read available translation tables */
	transl_load_all_tts();
	/* set up translations (for first time, will do it again after 
	loading config table, and then again after loading database */
	transl_load_xlats();

	/* check if any directories not specified, and try environment
	variables, and default to "." */
	for (i = 0; i < (int)ARRAYSIZE(dirvars); ++i) {
		String str = getenv(dirvars[i]);
		if (!str)
			str = ".";
		setoptstr_fallback(dirvars[i], str);
	}
	/* also check environment variable for editor */
	{
		String str = getenv("DEEDITOR");
#if defined(DEADENDS)
		if (! str)
		  str = getenv ("VISUAL");

		if (! str)
		  str = getenv ("EDITOR");

		if (! str)
		  str = "vi";
#else
		if (!str)
			str = environ_determine_editor(PROGRAM_LIFELINES);
#endif
		setoptstr_fallback("DEEDITOR", str);
	}
	/* editor falls back to platform-specific default */
	e = getdeoptstr("DEEDITOR", NULL);
	/* configure tempfile & edit command */
	editfile = environ_determine_tempfile();
	if (!editfile) {
		*pmsg = strsave("Error creating temp file");
		return false;
	}
	editfile = strsave(editfile );
	editstr = (String) stdalloc(strlen(e) + strlen(editfile) + 2);
	snprintf(editstr, strlen(e) + strlen(editfile) + 2, "%s %s", e, editfile);
	set_usersort(custom_sort);
	suppress_reload = false;
	update_useropts(0);

	return true;
}
/*=================================
 * init_lifelines_postdb -- 
 * Initialize stuff maintained in-memory
 *  which requires the database to already be opened
 *===============================*/
bool
init_lifelines_postdb (void)
{
	String emsg;
	StringTable *dbopts = createStringTable(NUMBER_OPTION_BUCKETS);

	tagtable = createStringTable(NUMBER_TAG_BUCKETS); /* values are same as keys */
	placabbvs = createStringTable(NUMBER_PLACABBV_BUCKETS);

	init_valtab_from_rec("VPLAC", placabbvs, ':', &emsg);
	init_valtab_from_rec("VUOPT", dbopts, '=', &emsg);
	set_db_options(dbopts);
	releaseHashTable(dbopts);
	init_browse_lists();
#if !defined(DEADENDS)
	if (!openxref(readonly))
		return false;
#endif
	transl_load_xlats();

	return true;
}
/*===================================
 * close_lifelines -- Close LifeLines
 *  Close entire lifelines engine - not just
 *  database (see close_lldb below).
 *=================================*/
void
close_lifelines (void)
{
#if !defined(DEADENDS)
	lldb_close(&def_lldb); /* make sure database closed */
#endif
	term_browse_lists();
#if !defined(DEADENDS)
	term_refnrec();
	term_namerec();
#endif
	if (editfile) {
		unlink(editfile);
		stdfree(editfile);
		editfile=NULL;
	}
	if (editstr) {
		stdfree(editstr);
		editstr=NULL;
	}
	term_lloptions();
#if !defined(DEADENDS)
	term_date();
#endif
	llgettext_term();
	term_codesets();
	strfree(&int_codeset);
	xlat_shutdown();
}
/*===================================================
 * is_codeset_utf8 -- Is this the name of UTF-8 ?
 *=================================================*/
bool
is_codeset_utf8 (CString codename)
{
	if (!codename || !codename[0]) return false;
	if (eqstr("UTF-8", codename)||eqstr("utf-8", codename)||eqstr("65001", codename))
		return true;
	return false;
}
/*===================================================
 * update_useropts -- Set any global variables
 * dependent on user options
 *=================================================*/
void
update_useropts (ATTRIBUTE_UNUSED void *uparm)
{
	if (suppress_reload)
		return;
	/* deal with db-specific options */
	/* includes setting int_codeset */
#if !defined(DEADENDS)
	if (def_lldb)
		update_db_options();
#endif
	/* in case user changed any codesets */
	init_codesets();
	/* in case user changed locale (need int_codeset already set) */
	uilocale();
	/* in case user changed codesets */
	/* TODO: Isn't this superfluous, as it was called in update_db_options above ? */
	transl_load_xlats();

	strupdate(&illegal_char, getdeoptstr("IllegalChar", 0));

#if !defined(DEADENDS)		/* XXX if we ever support nodechk we can revisit this XXX */
	nodechk_enable(!!getdeoptint("nodecheck", 0));
#endif
}
/*==================================================
 * update_db_options -- 
 *  check database-specific options for updates
 *================================================*/
static void
update_db_options (void)
{
	TABLE opttab = createStringTable(NUMBER_OPTION_BUCKETS);
	CString str=0;
	get_db_options(opttab);

	str = searchStringTable (opttab, "codeset");
	if (!str || !str[0]) {
		/*
		no specified database/internal codeset
		so default to user's default codeset, which
		should be from locale
		*/
		str = get_defcodeset();
	}
	if (!int_codeset)
		strupdate(&int_codeset, "");
	if (!eqstr_ex(int_codeset, str)) {
		/* database encoding changed */
		strupdate(&int_codeset, str);

		/* Here is where the global uu8 variable is set
		This is a flag if the internal (database) encoding is UTF-8 */
		uu8 = is_codeset_utf8(int_codeset);
		
		/* always translate to internal codeset */
		set_gettext_codeset(PACKAGE, int_codeset);
		/* need to reload all predefined codeset conversions */
		transl_load_xlats();
		if (uu8) {
			charprops_load_utf8();
		} else {
			charprops_load(int_codeset);
		}
	}

	deleteHashTable(opttab);
}
/*==================================================
 * pre_codesets_hook -- code to run just before initializing codesets
 * For MS-Windows user config of console codepages
 *================================================*/
static void
pre_codesets_hook (void)
{
#ifdef WIN32
	/* On MS-Windows, attempt to set any requested non-standard codepage */
	/* Do this now, before init_codesets below */
	int i = getdeoptint("ConsoleCodepage", 0);
	if (i) {
		w_set_oemout_codepage(i);
		w_set_oemin_codepage(i);
	}
#endif
}
/*==================================================
 * post_codesets_hook -- code to run just after initializing codesets
 * For Windows, link dynamically to gettext & iconv if available
 *================================================*/
static void
post_codesets_hook (void)
{
	init_win32_gettext_shim();
	init_win32_iconv_shim(getdeoptstr("iconv.path",""));
}
/*==================================================
 * load_configs -- Load global config file(s)
 * returns FALSE if error, with message in pmsg
 *================================================*/
static bool
load_configs (String configfile, String * pmsg)
{
	int rtn=0;
	String str=0;
	char cfg_name[MAXPATHLEN];

	/* TODO: Should read a system-wide config file */

	if (!configfile)
		configfile = getenv("DECONFIGFILE");

	*pmsg = NULL;


	if (configfile && configfile[0]) {

		rtn = load_global_options(configfile, pmsg);
		if (rtn == -1) return false;

	} else {

		/* No config file specified, so load config_file(s) from
		   the standard places */

		/* look for global config file */
		destrncpy(cfg_name, global_conf_path, sizeof(cfg_name), 0);
		destrapps(cfg_name, sizeof(cfg_name), 0, "/lifelines.conf");
		rtn = load_global_options(cfg_name, pmsg);
		if (rtn == -1) return false;

		/* look for one in user's home directory */
		/* TODO: Shouldn't Win32 use getenv("USERPROFILE") ? */
		destrncpy(cfg_name, getenv("HOME") , sizeof(cfg_name), 0);
		/*destrappc(cfg_name, sizeof(cfg_name), '/');*/
		destrapps(cfg_name, sizeof(cfg_name), 0, "/" DEADENDS_CONFIG_FILE);

		rtn = load_global_options(cfg_name, pmsg);
		if (rtn == -1) return false;

		rtn = load_global_options(DEADENDS_CONFIG_FILE, pmsg);
		if (rtn == -1) return false;
	}

	/* allow chaining to one more config file 
	 * if one was defined for the database 
	 */
	str = getdeoptstr("DECONFIGFILE", NULL);
	if (str && str[0]) {
		rtn = load_global_options(str, pmsg);
		if (rtn == -1) return false;
	}
	return true;
}
/*==================================================
 * check_installation_path -- Figure out installed path
 *================================================*/
static void
check_installation_path (void)
{
	int maxlen = sizeof(global_conf_path)-1;
#ifdef WIN32
	/* TODO: Installation should set value in registry
	and we should read it here */
	strncpy(global_conf_path, "C:\\Program Files\\lifelines", maxlen);
#else
	/* SYS_CONF_DIR was passed to make in src/gedlib/Makefile.am */
	strncpy(global_conf_path, SYS_CONF_DIR, maxlen);
#endif
	global_conf_path[maxlen] = 0;
}

/* 
   Copyright (c) 2000-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

/*==========================================================
 * lloptions.c -- Read options from config file (& db user options)
 *   added in 3.0.6 by Perry Rapp
 *========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "standard.h"
#include "denls.h"

#ifdef HAVE_LOCALE_H
#include <locale.h>
#endif

#include "gnode.h"
#include "stringtable.h"
#include "locales.h"
#include "codesets.h"
#include "listener.h"
#include "zstr.h"
#include "translat.h"
#include "xlat.h"
#include "readwrite.h"
#include "hashtable.h"
#include "list.h"
#include "lloptions.h"
#include "de-strings.h"
#include "messages.h"
#include "path.h"

//#if !defined(NUMBER_CONFIG_BUCKETS)
//#define NUMBER_CONFIG_BUCKETS	5
//#endif
#if !defined(NUMBER_FALLBACK_BUCKETS)
#define NUMBER_FALLBACK_BUCKETS	5
#endif
#if !defined(NUMBER_GLOBAL_BUCKETS)
#define NUMBER_GLOBAL_BUCKETS	37
#endif
#if !defined(NUMBER_PREDEF_BUCKETS)
#define NUMBER_PREDEF_BUCKETS	5
#endif
#if !defined(NUMBER_OPTION_BUCKETS)
#define NUMBER_OPTION_BUCKETS	53
#endif

//int num_config_buckets = NUMBER_CONFIG_BUCKETS;
int num_fallback_buckets = NUMBER_FALLBACK_BUCKETS;
int num_global_buckets = NUMBER_GLOBAL_BUCKETS;
int num_predef_buckets = NUMBER_PREDEF_BUCKETS;

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void copy_process(String dest, String src);
static void expand_variables(String valbuf, int max);
static int load_config_file(String file, String * pmsg, String *chain);
static void send_notifications(void);

/*********************************************
 * local variables
 *********************************************/

/* table holding option values, both keys & values in heap */
/* listed in descending priority */
static HashTable *f_cmd=0; /* option values from command line of current execution */
static HashTable *f_rpt=0; /* option values local to currently running report program */
	/* NB: cannot actually set any report option values currently 2002-10-07 */
static HashTable *f_db=0; /* option values in current database (user/options) */
static HashTable *f_global=0; /* option values from lines config file */
static HashTable *f_predef=0; /* predefined variables during config file processing */
static HashTable *f_fallback=0; /* lowest priority option values */
static List *f_notifications=0; /* collection of callbacks for option table changes */

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==========================================
 * copy_process -- copy config value line,
 *  converting any escape characters
 *  This handles \n, \t, and \\
 *  We do not trim out backslashes, unless they 
 *  are part of escape sequences. (This is mostly
 *  because backslashes are so prevalent in
 *  MS-Windows paths.)
 * The output (dest) is no longer than the input (src).
 * Created: 2001/11/09, Perry Rapp
 *========================================*/
static void
copy_process (String dest, String src)
{
	String q=dest,p=src;
	while ((*q = *p++)) {
		if (*q == '\\') {
			switch (*p++) {
			case 0:
				*++q = 0;
				break;
			case 'n':
				*q = '\n';
				break;
			case 't':
				*q  = '\t';
				break;
			case '\\':
				*q = '\\';
				break;
			default:
				--p;
			}
		}
		++q;
	}
}
/*==========================================
 * expand_variables -- do any variable substitutions
 *  (variables are option properties starting & ending with %
 * Created: 2002/10/21, Perry Rapp
 *========================================*/
static void
expand_variables (String valbuf, int max)
{
	String start, end;
	String ptr; /* remainder of valbuf to check */
	ptr = valbuf;
	while ((start=strchr(ptr, '%')) && (end=strchr(start+1, '%'))) {
		String name = allocsubbytes(start, 0, end-start+1);
		String value = searchStringTable(f_global, name);
		if (!value)
			value = searchStringTable(f_predef, name);
		if (value) {
			int newlen = strlen(valbuf)-(end-start+1)+strlen(value);
			if (newlen < max) {
				String copy = strdup(valbuf);
				if (start>valbuf)
					strncpy(valbuf, copy, start-valbuf);
				strcpy(start, value);
				strcpy(start+strlen(value), copy+(end-valbuf+1));
				stdfree(copy);
			}
		}
		stdfree(name);
		ptr = end+1;
	}
}
/*==========================================
 * dir_from_file -- return directory of file
 * heap-allocated
 *========================================*/
static String
dir_from_file (String file)
{
	String thisdir = strdup(file);
	String ptr;
	for (ptr=thisdir+strlen(thisdir)-1; ptr>thisdir; --ptr) {
		if (isDirSep(*ptr))
			break;
	}
	*ptr = 0;
	return thisdir;
}
/*==========================================
 * load_config_file -- read options in config file
 *  and load into table (f_global)
 * returns 1 for success, 0 for not found, -1 for error (with pmsg)
 *========================================*/
static int
load_config_file (String file, String * pmsg, String *chain)
{
	FILE * fp = 0;
	String ptr, val, key;
	String thisdir = dir_from_file(file);
	bool failed, noesc;
	char buffer[MAXLINELEN],valbuf[MAXLINELEN];
	int len;
	fp = fopen(file, DEREADTEXT);
	if (!fp) {
		stdfree(thisdir);
		return 0; /* 0 for not found */
	}
	f_predef = createStringTable(num_predef_buckets);

	addToStringTable(f_predef, "%thisdir%", thisdir);
	strfree(&thisdir);
	/* read thru config file til done (or error) */
	while (fgets(buffer, sizeof(buffer), fp)) {
		noesc = false;
		len = strlen(buffer);
		if (len == 0)
			continue; /* ignore blank lines */
		if (buffer[0] == '#')
			continue; /* ignore lines starting with # */
		if (!feof(fp) && buffer[len-1] != '\n') {
			/* bail out if line too long */
			break;
		}
		chomp(buffer); /* trim any trailing CR or LF */
		/* find =, which separates key from value */
		for (ptr = buffer; *ptr && *ptr!='='; ptr++)
			;
		if (*ptr != '=' || ptr==buffer)
			continue; /* ignore lines without = or key */
		*ptr=0; /* zero-terminate key */
		if (ptr[-1] == ':') {
			noesc = true; /* := means don't do backslash escapes */
			ptr[-1] = 0;
		}
		/* ignore any previous value, it will be overwritten */
		/* advance over separator to value */
		ptr++;
		/*
		process the value into valbuf
		this handles escapes (eg, "\n")
		the output (valbuf) is no longer than the input (ptr)
		*/
		if (noesc)
			destrncpy(valbuf, ptr, sizeof(valbuf), uu8);
		else
			copy_process(valbuf, ptr);
		expand_variables(valbuf, sizeof(valbuf));
		key = buffer; /* key is in beginning of buffer, we zero-terminated it */
		val = valbuf;
		if (strcmp(key,"LLCONFIGFILE") == 0) {
		    /* LLCONFIGFILE is not entered in table, only used to 
		     * chain to another config file
		     */
		    *chain = strsave(val);
		} else {
		    addToStringTable(f_global, key, val);
		}
	}
	failed = !feof(fp);
	fclose(fp);
	if (failed) {
		/* error is in heap */
		*pmsg = strsave(_(qSopt2long));
		return -1; /* -1 for error */
	}
	free_optable(&f_predef);
	send_notifications();
	return 1; /* 1 for ok */
}
/*=================================
 * load_global_options -- 
 *  Load internal table of global options from caller-specified config file
 * String * pmsg: heap-alloc'd error string if fails
 * returns 1 for ok, 0 for not found, -1 for error
 *===============================*/
int
load_global_options (String configfile, String * pmsg)
{
	String chain = NULL;
	int rtn = 0;
	int cnt = 0;
	*pmsg = NULL;
	if (!f_global) 
		f_global= createStringTable(num_global_buckets);
	do {
	    if (chain) strfree(&chain);
	    rtn = load_config_file(configfile, pmsg, &chain);
	    if (rtn == -1) {
		if (chain) strfree(&chain);
		return rtn;
	    }
	    if (++cnt > 100) {
	        return -1;  /* prevent infinite recursion */
	    }
	} while (chain);

	return rtn;
}
/*=================================
 * set_cmd_options -- Store cmdline options from caller
 *===============================*/
void
set_cmd_options (HashTable *opts)
{
	ASSERT(opts);
	releaseHashTable(f_cmd);
	f_cmd = opts;
	addrefHashTable(f_cmd);
	send_notifications();
}

/*=================================
 * set_db_options -- Store db options from caller
 * Created: 2002/06/16, Perry Rapp
 *===============================*/
void
set_db_options (HashTable *opts)
{
	ASSERT(opts);
	releaseHashTable(f_db);
	f_db = opts;
	addrefHashTable(f_db);
	send_notifications();
}

/*=================================
 * get_db_options -- Copy db options to caller's table
 * Created: 2002/06/16, Perry Rapp
 *===============================*/
void
get_db_options (HashTable *opts)
{
	if (!f_db)
		f_db = createStringTable(NUMBER_OPTION_BUCKETS);
	FORHASHTABLE(opts, element)
	  addToHashTable (f_db, element, true);
	ENDHASHTABLE
}

/*==========================================
 * free_optable -- free a table if it exists
 *========================================*/
void
free_optable (HashTable ** ptab)
{
	if (*ptab) {
		releaseHashTable(*ptab);
		*ptab = 0;
	}
}
/*==========================================
 * term_lloptions -- deallocate structures
 * used by lloptions at program termination
 * Safe to be called more than once
 * Created: 2001/04/30, Matt Emmerton
 *========================================*/
void
term_lloptions (void)
{
	free_optable(&f_cmd);
	free_optable(&f_rpt);
	free_optable(&f_db);
	free_optable(&f_global);
	free_optable(&f_fallback);
	remove_listeners(&f_notifications);
}
/*===============================================
 * getdeoptstr -- get an option (from db or from global)
 * Example: 
 *  str = getdeoptstr("HDR_SUBM", "1 SUBM");
 * returns string belonging to table
 *=============================================*/
String
getdeoptstr (CString optname, String defval)
{
	String str = 0;
	if (!str && f_cmd)
		str = searchStringTable(f_cmd, optname);
	if (!str && f_db)
		str = searchStringTable(f_db, optname);
	if (!str && f_global)
		str = searchStringTable(f_global, optname);
	if (!str && f_fallback)
		str = searchStringTable(f_fallback, optname);
	if (!str)
		str = defval;
	return str;
}
/*===============================================
 * getdeoptstr_rpt -- get an option (checking report-local options first)
 * Example: 
 *  str = getdeoptstr_rpt("HDR_SUBM", "1 SUBM");
 * Created: 2002/06/16, Perry Rapp
 *=============================================*/
String
getdeoptstr_rpt (CString optname, String defval)
{
	String str = 0;
	if (!str && f_rpt)
		str = searchStringTable(f_rpt, optname);
	if (!str)
		str = getdeoptstr(optname, defval);
	return str;
}
/*===============================================
 * getdeoptstr_dbonly -- get an option (but only look at db options)
 * Example: 
 *  str = getdeoptstr_dbonly("codeset", 0);
 * Created: 2002/06/16, Perry Rapp
 *=============================================*/
String
getdeoptstr_dbonly (CString optname, String defval)
{
	String str = 0;
	if (f_db)
		str = searchStringTable(f_db, optname);
	if (!str)
		str = defval;
	return str;
}
/*===============================================
 * getdeoptint -- get a numerical option
 *  First tries user option table (looks up optname)
 *  Then tries config option table
 *  Finally defaults to defval
 * Example: 
	if (getdeoptint("FullReportCallStack", 0) > 0)
 * Created: 2001/11/22, Perry Rapp
 *=============================================*/
int
getdeoptint (CString optname, int defval)
{
	String str = getdeoptstr(optname, 0);
	return str ? atoi(str) : defval;
}
/*===============================================
 * setoptstr_fallback -- Set option fallback value
 *=============================================*/
void
setoptstr_fallback (String optname, String newval)
{
	if (!f_fallback)
		f_fallback = createStringTable(num_fallback_buckets);
	addToStringTable(f_fallback, optname, newval);
	send_notifications();
}
/*===============================================
 * register_notify -- Caller wants to be notified when options change
 *=============================================*/
void
register_notify (CALLBACK_FNC fncptr)
{
	add_listener(&f_notifications, fncptr, 0);
}
/*===============================================
 * unregister_notify -- Caller no longer wants to be notified when options change
 *=============================================*/
void
unregister_notify (CALLBACK_FNC fncptr)
{
	delete_listener(&f_notifications, fncptr, 0);
}
/*===============================================
 * send_notifications -- Send notifications to any registered listeners
 *=============================================*/
static void
send_notifications (void)
{
	notify_listeners(&f_notifications);
}

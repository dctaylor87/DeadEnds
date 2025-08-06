/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * askgedc.c -- Construct list of GEDCOM files & prompt user
 *  Imitation of askprogram.c
 *==============================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(HAVE_DIRENT_H)
#include <dirent.h>
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "denls.h"
#include "sys_inc.h"

#include "rfmt.h"
#include "hashtable.h"
#include "refnindex.h"
#include "database.h"
#include "sequence.h"
#include "hashtable.h"
#include "stringtable.h"
#include "ask.h"
#include "errors.h"
#include "gedcom.h"
#include "liflines.h"
#include "messages.h"
#include "zstr.h"
#include "translat.h"
#include "xlat.h"
#include "readwrite.h"
#include "proptbls.h"
#include "de-strings.h"
#include "path.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void add_gedcom_props(HashTable *fileprops);
static void parse_gedcoms(HashTable ** fileprops);
static int select_gedcoms(const struct dirent *entry);
static void set_gedcom_d0(HashTable ** fileprops);

/*=========================
 * The supported meta-tags.
 *=======================*/

enum { P_SOUR=0, P_DATE=1 };

static CString f_tags[] = {
  "1 SOUR"
  , "1 DATE"
};


/*===========================================================
 * select_gedcoms -- choose files with the correct extention
 *==========================================================*/
static int
select_gedcoms (const struct dirent *entry)
{
	CString goodexts[] = { ".ged", ".gedcom" };
	int i;
	/* examine end of entry->d_name */
	for (i=0; i<(int)ARRAYSIZE(goodexts); ++i) {
		CString entext = entry->d_name + strlen(entry->d_name) - strlen(goodexts[i]);
		/* is it what we want ? use platform correct comparison, from path.c */
		if (path_match(goodexts[i], entext))
			return 1;
	}
	return 0;
}
/*==========================================================
 * add_gedcom_props -- set properties for gedcoms
 * Created: 2002/10/19, Perry Rapp
 *========================================================*/
static void
add_gedcom_props (HashTable *fileprops)
{
	String tagsfound[ARRAYSIZE(f_tags)];
	FILE *fp;
	char str[MAXLINELEN];
	int i;
	int line=0;
	struct stat sbuf;

	/* first get full path & open file */
	String fname = searchStringTable(fileprops, "filename");
	String dir = searchStringTable(fileprops, "dir");
	String filepath = pathConcatAllocate(dir, fname);

	if (!stat(filepath, &sbuf)) {
		if (sbuf.st_size > 9999999)
			snprintf(str, sizeof(str), "%ldMb", (long)(sbuf.st_size/1000000));
		else if (sbuf.st_size > 9999)
			snprintf(str, sizeof(str), "%ldKb", (long)(sbuf.st_size/1000));
		else
			snprintf(str, sizeof(str), "%ldb", (long)sbuf.st_size);

		add_prop_dnum(fileprops, "bytes", str);
	}

	if (NULL == (fp = fopen(filepath, DEREADTEXT)))
		goto end_add_program_props;

	/* initialize array where we record metainfo we want */
	for (i=0; i<(int)ARRAYSIZE(tagsfound); ++i)
		tagsfound[i] = 0;

	/* now read line by line looking for metainfo */
	while (NULL != fgets(str, sizeof(str), fp) && str[strlen(str)-1]=='\n' && line<20) {
		String p;
		chomp(str); /* trim trailing CR or LF */
		for (i=0; i<(int)ARRAYSIZE(f_tags); ++i) {
			CString tag = f_tags[i];
			if (tagsfound[i])
				continue; /* already have this tag */
			if (NULL != (p = strstr(str, tag))) {
				String s = p + strlen(tag);
				/* skip leading space */
				while (s[0] && isspace((u_char)s[0]))
					++s;
				striptrail(s);
				if (s[0])
					tagsfound[i] = strsave(s);
				break;
			}
		}
		if (!strncmp(str, "0 @", 3))
			break;
		++line;
	}

	fclose(fp);

	/* add any metainfo we found to the property table */
	for (i=0; i<(int)ARRAYSIZE(tagsfound); ++i) {
		if (tagsfound[i]) {
			add_prop_dnum(fileprops, f_tags[i], tagsfound[i]);
			strfree(&tagsfound[i]);
		}
	}

end_add_program_props:
	stdfree(filepath);
	return;
}
/*===================================================
 * parse_gedcoms -- parse gedcoms
 *  & record properties in property tables
 *  NB: must have NULL marker entry at end
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
static void
parse_gedcoms (HashTable ** fileprops)
{
	int i;
	for (i=0; fileprops[i]; ++i) {
		add_gedcom_props(fileprops[i]);
	}
}
/*===================================================
 * set_gedcom_d0 -- set display strings for gedcom files from property tables
 *  NB: must have NULL marker entry at end
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
static void
set_gedcom_d0 (HashTable ** fileprops)
{
	int i;
	for (i=0; fileprops[i]; ++i) {
		HashTable *props = fileprops[i];
		char buf[MAXLINELEN];
		String filename = searchStringTable(props, "filename");
		String sour = searchStringTable(props, (String)f_tags[P_SOUR]);
		String date = searchStringTable(props, (String)f_tags[P_DATE]);
		String bytes = searchStringTable(props, "bytes");
		if (!sour) sour="?";
		if (!date) date="?";
		if (!bytes) bytes="?";
		snprintf(buf, sizeof(buf), "%s (%s - %s) [%s]"
			, filename, date, bytes, sour);
		set_prop_dnum(props, 0, strsave("prompt"), strsave(buf));
	}
}
/*===================================================
 * ask_for_gedcom -- Ask for gedcom file
 *  pfname: [OUT]  allocated on heap (name we tried to open)
 *=================================================*/
bool
ask_for_gedcom (CString mode,
                 CString ttl,
                 String *pfname,
                 String path,
                 String ext,
                 bool picklist)
{
	int choice;
	int nfiles, i;
	HashTable ** fileprops;
	String * choices;
	FILE * fp;

	if (pfname) *pfname = 0;

	if (!picklist) {
		goto AskForString;
	}

	fileprops = get_proparray_of_files_in_path(path, select_gedcoms, &nfiles);
	parse_gedcoms(fileprops);
	set_gedcom_d0(fileprops);

	if (!nfiles) goto AskForString;

	choices = (String *)stdalloc(sizeof(String)*(nfiles+1));
	/* choices are all shared pointers */
	choices[0] = _(qSextchoos);
	for (i=0; i<nfiles; ++i) {
		choices[i+1] = searchStringTable(fileprops[i], searchStringTable(fileprops[i], "d0"));
	}
	choice = chooseFromArray_x(ttl, nfiles+1, choices, proparrdetails, fileprops);
	if (choice > 0) {
		HashTable *props = fileprops[choice-1];
		String fname = searchStringTable(props, "filename");
		//String dir = searchStringTable(props, "dir");
		//String filepath = pathConcatAllocate(dir, fname);
		if (pfname)
			*pfname = strsave(fname);
	}
	free_proparray(&fileprops);
	stdfree(choices);
	if (choice == 0) {
		/* 0th choice is to go to AskForString prompt instead */
		goto AskForString;
	}
	return (choice > 0);

AskForString:
	fp = ask_for_input_file(mode, ttl, pfname, path, ext);
	if (fp)
		fclose(fp);
	return fp != 0;
}

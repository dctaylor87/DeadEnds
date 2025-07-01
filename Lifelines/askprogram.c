/* 
   Copyright (c) 2001-2002 Petter Reinholdtsen & Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * askprogram.c -- Construct list of lifelines programs & prompt user
 *  Completely refactored 2002-10-19 to be usable for GUI
 *  and to be reusable for GEDCOM files (this code pushed into proptbls.c)
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

#include "hashtable.h"
#include "refnindex.h"
#include "gnode.h"
#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "xlat.h"
#include "readwrite.h"
#include "rfmt.h"
#include "database.h"
#include "sequence.h"
#include "hashtable.h"
#include "stringtable.h"
#include "ask.h"
#include "errors.h"
#include "liflines.h"
#include "messages.h"
#include "codesets.h"
#include "proptbls.h"
#include "de-strings.h"
#include "ui.h"
#include "path.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void add_program_props(HashTable *fileprops);
static void parse_programs(HashTable ** fileprops);
static int select_programs(const struct dirent *entry);
static void set_programs_d0(HashTable ** fileprops);

/*=========================
 * The supported meta-tags.
 *=======================*/

enum { P_PROGNAME=0, P_VERSION=1, P_OUTPUT=4 };

static CString f_tags[] = {
  "progname"    /* The name of the script */
  , "version"     /* The version of the script */
  , "author"      /* The author of the script */
  , "category"    /* The category of the script */
  , "output"      /* The output format produced by this script */
  , "description" /* A description of purpose of the script */
};


/*===========================================================
 * select_programs -- choose files with the correct extention
 *==========================================================*/
static int
select_programs (const struct dirent *entry)
{
	CString goodext = ".ll";
	/* examine end of entry->d_name */
	CString entext = entry->d_name + strlen(entry->d_name) - strlen(goodext);

	/* is it what we want ? use platform correct comparison, from path.c */
	if (!path_match(goodext, entext))
		return 0;

	return 1;
}
/*==========================================================
 * add_program_props -- set properties for programs (parse program file for metainfo)
 * Created: 2002/10/19, Perry Rapp
 *========================================================*/
static void
add_program_props (HashTable *fileprops)
{
	String tagsfound[ARRAYSIZE(f_tags)];
	FILE *fp;
	char str[MAXLINELEN];
	int i;
	int line=0;
	int endcomment=0; /* flag when finished header comment */
	char * charset=0; /* charset of report, if found */

	/* first get full path & open file */
	String fname = searchStringTable(fileprops, "filename");
	String dir = searchStringTable(fileprops, "dir");
	String filepath = pathConcatAllocate(dir, fname);
	char enc_cmd[] = "char_encoding(\"";

	if (NULL == (fp = fopen(filepath, DEREADTEXT)))
		goto end_add_program_props;

	/* initialize array where we record metainfo we want */
	for (i=0; i<(int)ARRAYSIZE(tagsfound); ++i)
		tagsfound[i] = 0;

	/* what charset is the data in this report ? :( */
	/* TODO: need to watch for BOM or for report charset command */

	/* now read line by line looking for metainfo */
	while (NULL != fgets(str, sizeof(str), fp) && str[strlen(str)-1]=='\n' && line<20) {
		/* check for char encoding command, to help us interpret tag values */
		if (!strncmp(str, enc_cmd, sizeof(enc_cmd)-1)) {
			const char *start = str+sizeof(enc_cmd)-1;
			const char *end = start;
			while (*end && *end!='\"')
				++end;
			if (*end && end>start) {
				charset=stdalloc(strlen(str));
				destrncpy(charset, start, end-start, 0);
			}
		}
		if (!endcomment) {
			/* pick up any tag values specified */
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
			if (strstr(str, "*/"))
				endcomment=1;
		}
		++line;
	}

	fclose(fp);

	/* add any metainfo we found to the property table */
	for (i=0; i<(int)ARRAYSIZE(tagsfound); ++i) {
		if (tagsfound[i]) {
			/* TODO: translate tagsfound[i] with charset */
			add_prop_dnum(fileprops, f_tags[i], tagsfound[i]);
			strfree(&tagsfound[i]);
		}
	}

end_add_program_props:
	strfree(&charset);
	stdfree(filepath);
	return;
}
/*===================================================
 * parse_programs -- parse lifelines report programs 
 *  & record properties in property tables
 *  NB: must have NULL marker entry at end
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
static void
parse_programs (HashTable ** fileprops)
{
	int i;
	for (i=0; fileprops[i]; ++i) {
		add_program_props(fileprops[i]);
	}
}
/*===================================================
 * set_programs_d0 -- set display strings for programs from property tables
 *  NB: must have NULL marker entry at end
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
static void
set_programs_d0 (HashTable ** fileprops)
{
	int i;
	for (i=0; fileprops[i]; ++i) {
		HashTable *props = fileprops[i];
		char buf[MAXLINELEN];
		String program = searchStringTable(props, (String)f_tags[P_PROGNAME]);
		String version = searchStringTable(props, (String)f_tags[P_VERSION]);
		String output = searchStringTable(props, (String)f_tags[P_OUTPUT]);
		if (!program)
			program = searchStringTable(props, "filename");
		if (!output)
			output = "?";
		if (!version)
			version = "V?.?";
		snprintf(buf, sizeof(buf), "%s (%s) [%s]"
			, program, version, output);
		set_prop_dnum(props, 0, strdup("prompt"), strdup(buf));
	}
}
/*===================================================
 * ask_for_program -- Ask for program
 *  pfname: [OUT]  allocated on heap (name we tried to open)
 *=================================================*/
bool
curses_ask_for_program (CString mode,
			CString ttl,
			String *pfname,
			CString path,
			CString ext,
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


	fileprops = get_proparray_of_files_in_path(path, select_programs, &nfiles);
	parse_programs(fileprops);
	set_programs_d0(fileprops);
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
		String dir = searchStringTable(props, "dir");
		String filepath = pathConcatAllocate(dir, fname);
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
/*================================================
 * proparrdetails -- print details of a file using proparray
 * Callback from chooseFromArray_x
 *  arrdets:  [IN]  package of list & current status
 *  param:    [IN]  the param we passed when we called chooseFromArray_x
 * Created: 2001/12/15 (Perry Rapp)
 *==============================================*/
void
proparrdetails (ARRAY_DETAILS arrdets, void * param)
{
	HashTable ** fileprops = (HashTable **)param;
	HashTable *props;
	int count = arrdets->count;
	int maxlen = arrdets->maxlen;
	int index = arrdets->cur - 1; /* slot#0 used by choose string */
	int row=0;
	int scroll = arrdets->scroll;
	int i;
	int dnum;

	if (index<0) return;

	props = fileprops[index];
	
	dnum = ll_atoi(searchStringTable(props, "dn"), 0);

	/* print tags & values */
	for (i=scroll; i<dnum && row<count; ++i, ++row) {
		String detail = arrdets->lines[row];
		char temp[FMT_INT_LEN+1];
		String name=0, value=0;
		snprintf(temp, sizeof(temp), "d" FMT_INT, i+1);
		name = searchStringTable(props, temp);
		detail[0]=0;
		if (name) {
			value = searchStringTable(props, name);
			destrapps(detail, maxlen, uu8, name);
			destrapps(detail, maxlen, uu8, ": ");
			if (value) {
				destrapps(detail, maxlen, uu8, value);
			}
		}
	}
  /* TODO: read file header */
}

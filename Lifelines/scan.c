/* 
   Copyright (c) 2000-2001 Perry Rapp

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
/*================================================================
 * scan.c -- Search database with full scan (several types)
 * Copyright(c) 2000-2001 by Perry Rapp; all rights reserved
 *   Created: 2000/12
 *==============================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(DEADENDS)
#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "llnls.h"
#include "sys_inc.h"

#include "rfmt.h"
#include "refnindex.h"
#include "sequence.h"
#include "uiprompts.h"
#include "feedback.h"
#include "errors.h"
#include "liflines.h"
#include "fpattern.h"
#include "messages.h"
#include "xreffile.h"
#include "refns.h"
#include "de-strings.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase
#else

#include "sys_inc.h"
#include <stdarg.h>
#include "llstdlib.h"
#include "table.h"
#include "indiseq.h"
#include "feedback.h"
#include "liflines.h"
#include "fpattern.h"
#include "messages.h"

#endif
#include "llinesi.h"

/*********************************************
 * local types
 *********************************************/

typedef struct
{
	int scantype;
	CString statusmsg;
	char pattern[64];
	INDISEQ seq;
	String field; /* field to scan, eg "AUTH" for sources by author */
	bool conts; /* include CONC & CONT children tags? */
} SCANNER;

/*********************************************
 * local enums
 *********************************************/

static int SCAN_NAME_FULL=0;
static int SCAN_NAME_FRAG=1;
static int SCAN_REFN=2;
static int SCAN_SRC_AUTH=3;
static int SCAN_SRC_TITL=4;

/*********************************************
 * local function prototypes
 *********************************************/

static void do_fields_scan(SCANNER * scanner, RecordIndexEl *rec);
static void do_name_scan(SCANNER * scanner, String prompt);
static void do_sources_scan(SCANNER * scanner, CString prompt);
static bool ns_callback(CString key, CString name, bool newset, void *param);
static bool rs_callback(CString key, CString refn, bool newset, void *param);
static void scanner_add_result(SCANNER * scanner, CString key);
static bool scanner_does_pattern_match(SCANNER *scanner, CString text);
static INDISEQ scanner_free_and_return_seq(SCANNER * scanner);
static void scanner_init(SCANNER * scanner, int scantype, CString statusmsg);
static void scanner_scan_titles(SCANNER * scanner);
static void scanner_set_field(SCANNER * scanner, String field);
static bool scanner_set_pattern(SCANNER * scanner, String pattern);

/*********************************************
 * local function definitions
 * body of module
 *********************************************/

/*==============================================
 * name_fragment_scan -- Ask for pattern and search all persons by name
 *  sts: [IN]  status to show during scan
 *============================================*/
INDISEQ
name_fragment_scan (CString sts)
{
	SCANNER scanner;
	scanner_init(&scanner, SCAN_NAME_FRAG, sts);
	do_name_scan(&scanner, _("Enter pattern to match against single surname or given name."));
	return scanner_free_and_return_seq(&scanner);
}
/*======================================
 * full_name_scan -- Ask for pattern and search all persons by full name
 *  sts: [IN]  status to show during scan
 *====================================*/
INDISEQ
full_name_scan (CString sts)
{
	SCANNER scanner;
	scanner_init(&scanner, SCAN_NAME_FULL, sts);
	do_name_scan(&scanner, _("Enter pattern to match against full name."));
	return scanner_free_and_return_seq(&scanner);
}
/*==============================
 * refn_scan -- Ask for pattern and search all refns
 *  sts: [IN]  status to show during scan
 *============================*/
INDISEQ
refn_scan (CString sts)
{
	SCANNER scanner;
	scanner_init(&scanner, SCAN_REFN, sts);

	while (1) {
		char request[MAXPATHLEN];
		String prompt = _("Enter pattern to match against refn.");
		bool rtn = ask_for_string(prompt, _("pattern: "),
			request, sizeof(request));
		if (!rtn || !request[0])
			return scanner_free_and_return_seq(&scanner);
		if (scanner_set_pattern(&scanner, request))
			break;
	}
	msg_status("%s", sts);
	traverse_refns(rs_callback, &scanner);

	return scanner_free_and_return_seq(&scanner);
}
/*==============================
 * scan_souce_by_author -- Ask for pattern and search all sources by author
 *  sts: [IN]  status to show during scan
 *============================*/
INDISEQ
scan_souce_by_author (CString sts)
{
	SCANNER scanner;
	scanner_init(&scanner, SCAN_SRC_AUTH, sts);
	scanner_set_field(&scanner, "AUTH");
	do_sources_scan(&scanner, _("Enter pattern to match against author."));
	return scanner_free_and_return_seq(&scanner);

}
/*==============================
 * scan_souce_by_title -- Ask for pattern and search all sources by title
 *  sts: [IN]  status to show during scan
 *============================*/
INDISEQ
scan_souce_by_title (CString sts)
{
	SCANNER scanner;
	scanner_init(&scanner, SCAN_SRC_TITL, sts);
	scanner_scan_titles(&scanner);
	do_sources_scan(&scanner, _("Enter pattern to match against title."));
	return scanner_free_and_return_seq(&scanner);
}
/*==============================
 * do_name_scan -- traverse names looking for pattern matching
 *  scanner:   [I/O] all necessary scan info, including sequence of results
 *  prompt:    [IN]  appropriate prompt to ask for pattern
 *============================*/
static void
do_name_scan (SCANNER * scanner, String prompt)
{
	while (1) {
		char request[MAXPATHLEN];
		bool rtn = ask_for_string(prompt, _("pattern: "),
			request, sizeof(request));
		if (!rtn || !request[0])
			return;
		if (scanner_set_pattern(scanner, request))
			break;
	}
	msg_status("%s", (String)scanner->statusmsg);
	traverse_names(ns_callback, scanner);
}
/*==============================
 * do_sources_scan -- traverse sources looking for pattern matching
 *  scanner:   [I/O] all necessary scan info, including sequence of results
 *  prompt:    [IN]  appropriate prompt to ask for pattern
 *============================*/
static void
do_sources_scan (SCANNER * scanner, CString prompt)
{
	int keynum = 0;

	while (1) {
		char request[MAXPATHLEN];
		bool rtn = ask_for_string(prompt, _("pattern: "),
			request, sizeof(request));
		if (!rtn || !request[0])
			return;
		if (scanner_set_pattern(scanner, request))
			break;
	}
	/* msg_status takes String arg, should take CString - const declaration error */
	msg_status("%s", (String)scanner->statusmsg);

	while (1) {
		RecordIndexEl *rec = 0;
		keynum = xref_nexts(keynum);
		if (!keynum)
			break;
		rec = keynum_to_srecord(keynum);
		do_fields_scan(scanner, rec);
	}
}
/*==============================
 * do_fields_scan -- traverse top nodes looking for desired field value
 *  scanner:   [I/O] all necessary scan info, including sequence of results
 *  rec:       [IN]  record to search
 *============================*/
static void
do_fields_scan (SCANNER * scanner, RecordIndexEl *rec)
{
	/* NB: Only scanning top-level nodes right now */
	GNode *node = nztop(rec);
	for (node = nchild(node); node; node = nsibling(node)) {
		String tag = ntag(node);
		if (tag && eqstr(tag, scanner->field)) {
			String val = nval(node);
			if (val && scanner_does_pattern_match(scanner, val)) {
				CString key = nzkey(rec);
				scanner_add_result(scanner, key);
				return;
			}
			if (scanner->conts) {
				GNode *node2 = 0;
				for (node2 = nchild(node); node2; node2 = nsibling(node2)) {
					String tag2 = ntag(node2);
					if (tag2 && (eqstr(tag2, "CONC") || eqstr(tag2, "CONT"))) {
						String val2 = nval(node2);
						if (val2 && scanner_does_pattern_match(scanner, val2)) {
							CString key = nzkey(rec);
							scanner_add_result(scanner, key);
							return;
						}
					} else {
						break;
					}
				}
			}
		}
	}
}
/*==============================
 * init_scan_pattern -- Initialize scan pattern fields
 *============================*/
static void
scanner_init (SCANNER * scanner, int scantype, CString statusmsg)
{
	scanner->scantype = scantype;
	scanner->seq = create_indiseq_null();
	strcpy(scanner->pattern, "");
	scanner->statusmsg = statusmsg;
	scanner->field = NULL;
	scanner->conts = false;
}
/*==============================
 * free_scanner_and_return_seq -- Free scanner data, except return result sequence
 *============================*/
static INDISEQ
scanner_free_and_return_seq (SCANNER * scanner)
{
	strfree(&scanner->field);
	return scanner->seq;
}
/*=================================================
 * set_pattern -- Store scanner pattern (or return FALSE if invalid)
 *===============================================*/
static bool
scanner_set_pattern (SCANNER * scanner, String str)
{
	int i;
	/* spaces don't make sense in a name fragment */
	if (scanner->scantype == SCAN_NAME_FRAG) {
		for (i=0; str[i]; i++)
			if (str[i] == ' ')
				return false;
	}

	if (!fpattern_isvalid(str))
		return false;

	if (strlen(str) > sizeof(scanner->pattern)-1)
		return false;

	strcpy(scanner->pattern, str);

	return true;
}
/*=================================================
 * scanner_set_field -- Search specified field
 *===============================================*/
static void
scanner_set_field (SCANNER * scanner, String field)
{
	strupdate(&scanner->field, field);
}

/*=================================================
 * scanner_scan_titles -- Search titles (& CONT & CONC children)
 *===============================================*/
static void
scanner_scan_titles (SCANNER * scanner)
{
	strupdate(&scanner->field, "TITL");
	scanner->conts = true;
}
/*=============================================
 * scanner_does_pattern_match -- Compare a name to a pattern
 *===========================================*/
static bool
scanner_does_pattern_match (SCANNER *scanner, CString text)
{
	return (fpattern_matchn(scanner->pattern, text));
}
/*=============================================
 * scanner_add_result -- Add a hit to the result list
 *===========================================*/
static void
scanner_add_result (SCANNER * scanner, CString key)
{
	/* if we pass in name, append_indiseq won't check for dups */
	append_indiseq_null(scanner->seq, strsave(key), NULL, false, true);
}
/*===========================================
 * ns_callback -- callback for name traversal
 *=========================================*/
static bool
ns_callback (CString key, CString name, ATTRIBUTE_UNUSED bool newset, void *param)
{
	int len, ind;
	String piece;
	SCANNER * scanner = (SCANNER *)param;
	if (scanner->scantype == SCAN_NAME_FULL) {
		if (scanner_does_pattern_match(scanner, name)) {
			scanner_add_result(scanner, key);
		}
	} else {
		/* SCAN_NAME_FRAG */
		List *list = name_to_list(name, &len, &ind);
		FORLIST(list, el)
			piece = (String)el;
			if (scanner_does_pattern_match(scanner, piece)) {
				scanner_add_result(scanner, key);
#if !defined(DEADENDS)
				STOPLIST
#endif
				break;
			}
		ENDLIST
		destroy_list(list);
	}
	return true;
}
/*===========================================
 * rs_callback -- callback for refn traversal
 *=========================================*/
static bool
rs_callback (CString key, CString refn, ATTRIBUTE_UNUSED bool newset, void *param)
{
	SCANNER * scanner = (SCANNER *)param;
	ASSERT(scanner->scantype == SCAN_REFN);

	if (scanner_does_pattern_match(scanner, refn)) {
		scanner_add_result(scanner, key);
	}
	return true;
}

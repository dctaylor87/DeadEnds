/* 
   Copyright (c) 2000-2002 Perry Rapp
   Copyright (c) 2003 Matt Emmerton
   "The MIT license"

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * ui_cli.c -- UI code for command-line interface (CLI), *not* curses
 * Copyright(c) 2002 by Perry Rapp; all rights reserved
 * Copyright(c) 2003 by Matt Emmerton; all rights reserved
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "standard.h"
#include "denls.h"
#include "sequence.h"
#include "feedback.h"
#include "readwrite.h"
#include "messages.h"
#include "codesets.h"
#include "de-strings.h"
#include "ui.h"

/*********************************************
 * external variables (no header)
 *********************************************/

/* XXX default value -- for now; need to ask OS for value; might also
   want to change if user resizes the window.  XXX */
int screen_width = 80;

/*********************************************
 * local function prototypes
 *********************************************/
static void outputln(const char * txt);
static void output(const char * txt);
static int interact(CString ptrn);

static int
chooseOrViewArray (CString ttl, int no, String *pstrngs, bool selectable);
/*=============================================================
 * Xprintf() implementations
 *===========================================================*/

void
llvwprintf (CString fmt, va_list args)
{
	vprintf(fmt, args);
}
/*=============================================================
 * Report output functions
 *===========================================================*/

void
rpt_print (CString str)
{
	printf("%s", str);
}

void
refresh_stdout (void)
{
	/* We don't need to do anything as we're using stdout */
}

/*=============================================================
 * Message output functions
 *===========================================================*/

int
msg_width (void)
{
	/* arbitrarily high number */
	return 999;
}

/*=============================================================
 * MTE: this really belongs in stdlib/
 *===========================================================*/

void
call_system_cmd (CString cmd)
{
	int rtn=-1;

#ifndef WIN32
	rtn = system("clear");
#endif
	rtn = system(cmd);

	if (rtn != 0) {
		printf(_("Editor or system call failed."));
		puts("");
		sleep(2);
        }
}

/*=============================================================
 * ASK Routines
 *===========================================================*/

bool
ask_for_program (ATTRIBUTE_UNUSED CString mode,
                 ATTRIBUTE_UNUSED CString ttl,
                 ATTRIBUTE_UNUSED String *pfname,
                 ATTRIBUTE_UNUSED String *pfullpath,
                 ATTRIBUTE_UNUSED CString path,
                 ATTRIBUTE_UNUSED CString ext,
                 ATTRIBUTE_UNUSED bool picklist)
{
	/* TODO: We probably want to use the real implementation in askprogram.c */
	return false;
}

bool
ask_for_string (CString ttl, CString prmpt, String buffer, int buflen)
{
	char *rtn=NULL;
	int len=0;

	outputln(ttl);
	printf("%s", prmpt);
	rtn = fgets(buffer, buflen, stdin);
	if (rtn)
	{
		chomp(buffer);
		len = strlen(buffer);
	}

	return (len>0);
}

bool
ask_for_string2 (CString ttl1, CString ttl2, CString prmpt, String buffer, int buflen)
{
	outputln(ttl1);
	return ask_for_string(ttl2, prmpt, buffer, buflen);
}

int
ask_for_char_msg (CString msg, CString ttl, CString prmpt, CString ptrn)
{
	int rv;
	if (msg) outputln(msg);
	if (ttl) outputln(ttl);
	output(prmpt);
	rv = interact(ptrn);
	return rv;
}

bool
ask_for_filename_impl (CString ttl, CString path, CString prmpt, String buffer, int buflen)
{
	/* display current path (truncated to fit) */
	char curpath[120];
	int len = sizeof(curpath);
	if (len > screen_width-2)
		len = screen_width-2;
	curpath[0] = 0;
	destrapps(curpath, len, uu8, _(qSiddefpath));
	destrapps(curpath, len, uu8, compressPath(path, len-strlen(curpath)-1));

	return ask_for_string2(ttl, curpath, prmpt, buffer, buflen);
}

/*=============================================================
 * CHOOSE Routines
 *===========================================================*/

int
chooseFromArray (CString ttl, int no, String *pstrngs)
{
	bool selectable = true;
	return chooseOrViewArray(ttl, no, pstrngs, selectable);
}

void
view_array (CString ttl, int no, String *pstrngs)
{
	bool selectable = false;
	chooseOrViewArray(ttl, no, pstrngs, selectable);
}

int
chooseOneOrListFromSequence (ATTRIBUTE_UNUSED CString ttl, Sequence *seq, ATTRIBUTE_UNUSED bool multi)
{
#if !defined(DEADENDS)	 /* DEADENDS always fills in names for a person sequence */
	calc_indiseq_names(seq); /* we certainly need the names */
#endif

	/* TODO: imitate chooseFromList & delegate to array chooser */
	return 0;
}

static int
chooseOrViewArray (CString ttl, int no, String *pstrngs, bool selectable)
{
	String promptline = selectable ? _(qSchlistx) : _(qSvwlistx);
	String responses = selectable ? "0123456789udq" : "udq";

	int start=1;
	while (1) {
                int end = start+(start == 1 ? 8 : 9);
                if (end > no) {
                        end = no;
                }
		int j;
		int rv;
                printf("%s (" FMT_INT "/" FMT_INT ")\n", _(ttl),start,no);

		for (j=start; j<=end; ++j) {
			printf(FMT_INT ": %s\n", j%10, pstrngs[j-1]);
		}
		printf("%s\n", promptline);
		rv = interact(responses);
		switch(rv) {
		case '0':
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			rv = rv-'1' + (start/10)*10;
			if (selectable && rv < no ) {
				return rv;
			}
			break;
		case 'd':
                        // if end == no don't slide window down
                        if (end != no) {
                                if (start == 1) {
                                        start += 9;
                                } else {
                                        start += 10;
                                }
                                if (start > no) {
                                         start = no;
                                }
                        }
			break;
		case 'u':
			if (start >9)  {
                                start -= 10;
                        }
                        if (start < 1) {
                                start = 1;
                        }
			break;
                case 0: /* trap EOF and treat like a q */
		case 'q': return -1;
		}
	}
}

/*=============================================================
 * Misc Routines
 *===========================================================*/

int
prompt_stdout (CString prompt)
{
	return ask_for_char(NULL, prompt, NULL);
}

/* called from ask.c, curses version in searchui.c */
Sequence *
invoke_search_menu (void)
{
	return NULL;
}

/*=============================================================
 * Internal Use Only
 *===========================================================*/

/* send string to output, & terminate line */
static void
outputln (const char * txt)
{
	printf("%s", txt);
	printf("\n");
}

/* send string to output */
static void
output (const char * txt)
{
	printf("%s", txt);
}

static int
interact (CString ptrn)
{
	char buffer[8];
	CString t=0;
	char *rtn=NULL;

	while (1) {
		rtn = fgets(buffer, sizeof(buffer), stdin);
		if (!rtn) return 0;
		if (!ptrn) return buffer[0];
		for (t=ptrn; *t; ++t) {
			if (buffer[0]==*t)
				return buffer[0];
		}
		printf("Invalid option(%c): choose one of %s\n",buffer[0], ptrn);
	}
}

/* 
   Copyright (c) 2003 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * rptui.c -- Wrappers for UI functions used by report interpreter
 * These take care of switching to UI locale, and keeping GUI wait time
 *  accounted separately from report run time.
 *==============================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <stdint.h>
#include <stdio.h>
#include <time.h>

#include "standard.h"
#include "recordindex.h"
#include "sequence.h"
#include "locales.h"
#include "ui.h"
#include "ask.h"
#include "rptui.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static void begin_rptui(void);
static void end_rptui(void);

/*********************************************
 * local variables
 *********************************************/

static time_t uitime=0;
static time_t begint=0;

/*=================================================
 * begin_rptui -- begin a UI call from report interpreter
 *===============================================*/
static void
begin_rptui (void)
{
	uilocale();
	begint = time(NULL);
}
/*=================================================
 * end_rptui -- finish a UI call & return to report interpreter
 *===============================================*/
static void
end_rptui (void)
{
	rptlocale();
	uitime += time(NULL) - begint;
	begint = 0;
}
/*=================================================
 * rptui_init -- begin UI timing for report
 *===============================================*/
void
rptui_init (void)
{
	uitime = 0;
}
/*=================================================
 * rptui_elapsed -- report elapsed UI time for report
 *===============================================*/
int
rptui_elapsed (void)
{
	return (int)uitime;
}
/*==========================================================
 * Wrappers for ui functions
 *========================================================*/

GNode *
rptui_ask_for_fam (CString s1, CString s2)
{
	GNode *rec;
	begin_rptui();
	rec = ask_for_fam(s1, s2);
	end_rptui();
	return rec;
}

Sequence *
rptui_ask_for_indi_list (CString ttl, bool reask)
{
	Sequence *seq;
	begin_rptui();
	seq = ask_for_indi_list(ttl, reask);
	end_rptui();
	return seq;
}

GNode *
rptui_ask_for_indi (CString ttl, ASK1Q ask1)
{
	GNode *rec;
	begin_rptui();
	rec = ask_for_indi(ttl,ask1);
	end_rptui();
	return rec;
}

CString
rptui_ask_for_indi_key (CString ttl, ASK1Q ask1)
{
	CString s;
	begin_rptui();
	s = ask_for_indi_key(ttl, ask1);
	end_rptui();
	return s;
}
bool
rptui_ask_for_int (CString ttl, int * prtn)
{
	bool b;
	begin_rptui();
	b = ask_for_int(ttl, prtn);
	end_rptui();
	return b;
}
FILE *
rptui_ask_for_output_file (CString mode, CString ttl, String *pfname
	, String *pfullpath, CString path, CString ext)
{
	FILE * fp;
	begin_rptui();
	fp = ask_for_output_file(mode, ttl, pfname, pfullpath, path, ext);
	end_rptui();
	return fp;
}
bool
rptui_ask_for_program (CString mode, CString ttl, String *pfname
	, String *pfullpath, CString path, CString ext, bool picklist)
{
	bool b;
	begin_rptui();
	b = ask_for_program(mode, ttl, pfname, pfullpath, path, ext, picklist);
	end_rptui();
	return b;
}
int
rptui_chooseFromArray (CString ttl, int no, String *pstrngs)
{
	int i;
	begin_rptui();
	i = chooseFromArray(ttl, no, pstrngs);
	end_rptui();
	return i;
}
int
rptui_prompt_stdout (CString prompt)
{
	int i;
	begin_rptui();
	i = prompt_stdout(prompt);
	end_rptui();
	return i;
}
void
rptui_view_array (CString ttl, int no, String *pstrngs)
{
	begin_rptui();
	view_array(ttl, no, pstrngs);
	end_rptui();
}

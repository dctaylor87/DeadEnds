/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*==============================================================
 * feedback.h -- Header file for I/O feedback needed by non-ui code
 *  NB: record oriented functions required are in liflines.h
 * Copyright (c) 2001-2002 by Perry Rapp; all rights reserved
 *============================================================*/

#ifndef _FEEDBACK_H
#define _FEEDBACK_H

#ifndef INCLUDED_STDARG_H
#include <stdarg.h>
#define INCLUDED_STDARG_H
#endif


/* Ways for engine & code to report to ui */
	/* report an error */
void msg_error(String fmt, ...) HINT_PRINTF(1,2);
	/* report a message */
void msg_info(String fmt, ...) HINT_PRINTF(1,2);
	/* report transitory state that should not be preserved */
void msg_status(String fmt, ...) HINT_PRINTF(1,2);
	/* more longwinded ways */
typedef enum { MSG_ERROR=-1, MSG_INFO, MSG_STATUS } MSG_LEVEL;
void msg_output(MSG_LEVEL, String fmt, ...) HINT_PRINTF(2,3);
void msg_outputv(MSG_LEVEL, String fmt, va_list args);
	/* legacy */
	/* message () is a macro -- does not localize */
void message(String fmt, ...) HINT_PRINTF(1,2);
	/* report to stdout style output (uses embedded carriage returns */
void llwprintf(String fmt, ...) HINT_PRINTF(1,2);
void llvwprintf(String fmt, va_list args);
	/* how many characters available for msg_xxx strings (-1 if unlimited) */
int msg_width(void);
	/* report language print function */
void rpt_print(String str);

/* called by ask.c */
bool ask_for_input_filename(String ttl, String path, String prmpt, String buffer, int buflen);
bool ask_for_output_filename(String ttl, String path, String prmpt, String buffer, int buflen);

/* called by signal handler before invoking exit() */
void shutdown_ui(bool pause);

/* called by edit routines for translation maps &
 edit routines for tables for user options & abbreviations */
void do_edit(void);

/* msg boxes */
bool ask_yes_or_no(String ttl);
bool ask_yes_or_no_msg(String msg, String ttl);
bool ask_for_string(CString ttl, CString prmpt, String buffer, int buflen);
bool ask_for_string2(CString ttl1, CString ttl2, CString prmpt, String buffer, int buflen);

/* called by interp when finished */
void refresh_stdout(void);

/* for report interpreter engine */
void call_system_cmd(String cmd);

#endif /* _FEEDBACK_H */


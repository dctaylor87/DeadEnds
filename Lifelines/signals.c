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
 * signals.c -- Catch signals and exit
 * Copyright(c) 1993-94 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   2.3.4 - 24 Jun 93    2.3.5 - 01 Sep 93
 *   3.0.0 - 30 Jul 94    3.0.2 - 10 Nov 94
 *   3.0.3 - 19 Sep 95
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "sys_inc.h"
#include "standard.h"
#include "errors.h"
#include "denls.h"

#include "zstr.h"
#include "list.h"
#include "translat.h"
#include "interp.h"
#include "ask.h"
#include "liflines.h"
#include "feedback.h"
#include "messages.h"
#include "signals.h"

/*********************************************
 * local function prototypes
 *********************************************/

/*********************************************
 * local variables
 *********************************************/
static char *sig_msgs[21];

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*======================================
 * load_signames -- Load descriptive signal names
 *====================================*/
void
load_signames (void)
{
	sig_msgs[ 0] = _(qSsig00);
	sig_msgs[ 1] = _(qSsig01);
	sig_msgs[ 2] = _(qSsig02);
	sig_msgs[ 3] = _(qSsig03);
	sig_msgs[ 4] = _(qSsig04);
	sig_msgs[ 5] = _(qSsig05);
	sig_msgs[ 6] = _(qSsig06);
	sig_msgs[ 7] = _(qSsig07);
	sig_msgs[ 8] = _(qSsig08);
	sig_msgs[ 9] = _(qSsig09);
	sig_msgs[10] = _(qSsig10);
	sig_msgs[11] = _(qSsig11);
	sig_msgs[12] = _(qSsig12);
	sig_msgs[13] = _(qSsig13);
	sig_msgs[14] = _(qSsig14);
	sig_msgs[15] = _(qSsig15);
	sig_msgs[16] = _(qSsig16);
	sig_msgs[17] = _(qSsig17);
	sig_msgs[18] = _(qSsig18);
	sig_msgs[19] = _(qSsig19);
	sig_msgs[20] = _(qSsig20);
}
/*======================================
 * set_signals -- Install signal handler
 *====================================*/
void
set_signals (void (*handler)(int))
{
	if (signal(SIGINT, SIG_IGN) != SIG_IGN)
		signal(SIGINT, handler);
#ifdef SIGHUP
	signal(SIGHUP, handler);
#endif
#ifdef SIGQUIT
	signal(SIGQUIT, handler);
#endif
	signal(SIGILL, handler);
#ifdef SIGEMT
	signal(SIGEMT, handler);
#endif
	signal(SIGFPE, handler);
#ifdef SIGBUS
	signal(SIGBUS, handler);
#endif
	signal(SIGSEGV, handler);
#ifdef SIGSYS
	signal(SIGSYS, handler);
#endif
#ifdef SIGPIPE
	signal(SIGPIPE, handler);
#endif
}
/*======================================
 * get_signame -- Get signal name
 *====================================*/
char *
get_signame (int sig)
{
  char *signame;

  if ((sig >= 0) && (sig < (int)ARRAYSIZE(sig_msgs)))
    signame = sig_msgs[sig];
  else
    signame = _(qSsigunk);

  return signame;
}

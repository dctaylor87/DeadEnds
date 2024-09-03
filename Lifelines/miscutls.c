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
 * miscutils.c -- Miscellaneous utility commands
 * Copyright(c) 1992-94 by T.T. Wetmore IV; all rights reserved
 *   2.3.4 - 24 Jun 93    2.3.5 - 15 Aug 93
 *   2.3.6 - 12 Oct 93    3.0.0 - 05 May 94
 *   3.0.2 - 08 Nov 94
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "denls.h"

#include "list.h"
#include "zstr.h"
#include "translat.h"
#include "feedback.h"
#include "refnindex.h"
#include "gnode.h"
#include "recordindex.h"
#include "rfmt.h"
#include "sequence.h"
#include "uiprompts.h"
#include "lineage.h"
#include "llinesi.h"
#include "errors.h"
#include "liflines.h"
#include "messages.h"

/* everything in this file assumes we are dealing with the current database */
#define database	currentDatabase

/*======================================
 * key_util -- Return person's key value
 *====================================*/
void
key_util (void)
{
	RecordIndexEl *indi = ask_for_indi(_("Whose key value do you want?"), NOASK1);
	if (!indi) return;
	msg_info("%s - %s", rmvat(nxref(nztop(indi))), personToName(nztop(indi), 70));
}

#if !defined(DEADENDS)
/*===================================================
 * who_is_he_she -- Find who person is from key value
 *=================================================*/
void
who_is_he_she (void)
{
	String str, rawrec;
	GNode *indi;
	int len;
	char nkey[100];
	char key[20];

	if (!ask_for_string(_("Please enter person's internal key value."),
	    _("enter key:"), key, sizeof(key))
		 || !key[0])
		 return;

	nkey[0] = 'I';
	if (*key == 'I')
		strcpy(nkey, key);
	else
		strcpy(&nkey[1], key);
	if (!(rawrec = retrieve_raw_record(nkey, &len))) {
		msg_error(_("No one in database has key value %s."), key);
		return;
	}
	if (!(indi = stringToNodeTree(rawrec))) {
		msg_error(_("No one in database has key value %s."), key);
		stdfree(rawrec);
		return;
	}
	if (!(str = personToName(indi, 60)) || *str == 0) {
		msg_error(_("No one in database has key value %s."), key);
		stdfree(rawrec);
		return;
	}
	msg_info("%s - %s", key, str);
	/* LEAK -- where is stdfree(rawrec) -- Perry 2001/11/18 */
}
#endif

/*===========================================
 * show_database_stats -- Show database stats
 *=========================================*/
void
show_database_stats (void)
{
	char msg[80];
	snprintf(msg, sizeof(msg), "%s", _(qSdbrecords));
	strcat(msg, ": ");
	snprintf(msg+strlen(msg), sizeof(msg)-strlen(msg)
		, _(qSdbrecstats), num_indis(), num_fams()
		, num_sours(), num_evens(), num_othrs());
	msg_info("%s", msg);
}

/*======================================
 * sighand_cursesui -- Catch and handle signal (UI)
 *====================================*/

void
sighand_cursesui(int sig)
{
  char *sig_desc = strsignal (sig); /* localized signal description */
  char *sig_format = _(qSsignal);   /* localized format string */
  size_t len = strlen(sig_desc) + strlen(sig_format) + 20; /* 20 is overkill */
  char abort_msg[len];

  close_lifelines();
  shutdown_ui(true); /* pause */

  showErrorLog(globalErrorLog);

  /* build the message for ll_optional_abort to display */
  snprintf (abort_msg, len, sig_format, sig, sig_desc);
  ll_optional_abort(abort_msg);

  exit(1);
}

/*======================================
 * sighand_cmdline - Catch and handle signal cleanly (command-line)
 *====================================*/
void
sighand_cmdline(int sig ATTRIBUTE_UNUSED)
{
  exit(1);
}

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
 * strwhite.c -- Functions to remove whitespace from strings
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(DEADENDS)
#include <ansidecl.h>

#include "porting.h"
#include "standard.h"
#include "llnls.h"

#include "de-strings.h"

#else

#include "llstdlib.h"

#endif

#if !defined(DEADENDS)		/* now part of Utils/standard.c */
/*================================
 * trim -- Trim string if too long
 *  returns static buffer (or NULL)
 *==============================*/
String
trim (String str, int len)
{
	static char scratch[MAXLINELEN+1];
	if (!str || strlen(str) > MAXLINELEN) return NULL;
	if (len < 0) len = 0;
	if (len > MAXLINELEN) len = MAXLINELEN;
	strcpy(scratch, str);
	scratch[len] = '\0';
	return scratch;
}
/*=========================================
 * striptrail -- Strip trailing white space
 *  modifies argument (zeros out trailing whitespace)
 *=======================================*/
void
striptrail (String p)
{
	String q = p + strlen(p) - 1;
	while (q >= p && iswhite((u_char)*q))
		*q-- = '\0';
}
#endif

#ifdef UNUSED_CODE
/*=======================================
 * striplead -- Strip leading white space
 *  modifies argument (shifts up string towards
 *  beginning to eliminate any leading whitespace)
 * UNUSED CODE
 *=====================================*/
void
striplead (String p)
{
	int i = strlen(p);
	String  e = p + i - 1;
	String b = p;
	String q = p;

	while (iswhite((u_char)*q) && q <= e) {
		++q;
		--i; /* keep from copying past end of p */
	}
	if (q == p) return;

	while (b <= e && --i >= 0)
		*b++ = *q++;
	*b++ = '\0';
}
#endif /* UNUSED_CODE */
/*=========================================
 * skipws -- Advance pointer over whitespace
 *=======================================*/
void
skipws (String * ptr)
{
	while (iswhite(*(u_char *)(*ptr)))
		++(*ptr);
}

#if !defined(DEADENDS)		/* now part of Utils/standard.c */
/*=========================================
 * allwhite -- Check if string is all white
 *=======================================*/
bool
allwhite (String p)
{
	while (*p)
		if (!iswhite((u_char)*p++)) return false;
	return true;
}
#endif

/*============================================
 * chomp -- remove any trailing carriage return/linefeed
 * Created: 2002/01/03 (Perry Rapp)
 *==========================================*/
void
chomp (String str)
{
	String p = str + strlen(str) - 1;
	while (p>=str && (*p=='\r' || *p=='\n')) {
		*p=0;
		--p;
	}
}

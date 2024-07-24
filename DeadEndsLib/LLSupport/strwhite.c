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

#include <ansidecl.h>

#include "porting.h"
#include "standard.h"
#include "llnls.h"

#include "de-strings.h"

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

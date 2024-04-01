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
 * strcvt.c -- string conversion functions
 *===========================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#ifdef HAVE_WCHAR_H
#include <wchar.h>
#endif
#ifdef HAVE_WCTYPE_H
#include <wctype.h>
#endif

#if defined(DEADENDS)
#include <ansidecl.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "sys_inc.h"
#include "llnls.h"

#include "zstr.h"
#include "icvt.h"
#include "stdlibi.h"
#include "codesets.h"
#include "de-strings.h"
#include "strcvt.h"

#else

#include "llstdlib.h" /* includes standard.h, sys_inc.h, llnls.h, config.h */

#include "zstr.h"
#include "icvt.h"
#include "stdlibi.h"

#endif
static const char * get_wchar_codeset_name(void);
static ZSTR (*upperfunc)(CString) = 0;
static ZSTR (*lowerfunc)(CString) = 0;

/*===================================================
 * get_wchar_codeset_name -- name of wchar_t codeset
 * (handles annoyance that MS-Windows wchar_t is small)
 *=================================================*/
static const char *
get_wchar_codeset_name (void)
{
	if (sizeof(wchar_t)==2) {
	/* MS-Windows can't handle UCS-4; could we use UTF-16 ? */
		return "UCS-2-INTERNAL";
	} else if (sizeof(wchar_t)==4) {
		return "UCS-4-INTERNAL";
	} else {
		ASSERT(0);
	}
}
/*===================================================
 * makewide -- convert internal to wchar_t *
 * handling annoyance that MS-Windows wchar_t is small
 * Either returns 0 if fails, or a zstring which actually
 * contains wchar_t characters.
 *=================================================*/
ZSTR
makewide (const char *str)
{
	ZSTR zstr=0;
	if (int_codeset && int_codeset[0]) {
		CString dest = get_wchar_codeset_name();
		zstr = zs_new();
		/* dest = "wchar_t" doesn't work--Perry, 2002-11-20 */
		if (!iconv_trans(int_codeset, dest, str, zstr, '?')) {
			zs_free(&zstr);
		}
	}
	return zstr;
}
/*===================================================
 * makeznarrow -- Inverse of makewide
 *  Created: 2002-12-15 (Perry Rapp)
 *=================================================*/
ZSTR
makeznarrow (ZSTR zwstr)
{
	ZSTR zout=zs_new();
	CString src = get_wchar_codeset_name();
	if (!iconv_trans(src, int_codeset, zs_str(zwstr), zout, '?'))
		zs_free(&zout);
	return zout;
}
/*=========================================
 * isnumeric -- Check string for all digits
 * Used to parse GEDCOM pointers (eg, @I23@)
 * so only needs to handle ASCII digits
 * Does not need to handle any other types of digits
 *=======================================*/
bool
isnumeric (String str)
{
	int c;
	if (!str) return false;
	/* only needs to handle ASCII digits
	so can use very simple ASCII comparison */
	while ((c = (u_char)*str++)) {
		if (!(c >= '0' && c <= '9'))
			return false;
	}
	return true;
}
/*======================================
 * iswletter -- is widechar a letter ?
 *====================================*/
bool
iswletter (wchar_t wch)
{
#ifdef HAVE_ISWALPHA
	return iswalpha(wch);
#else
	unsigned int n = wch;
	return (n>='a' && n<='z') || (n>='A' && n<='Z');
#endif
}
#ifdef HAVE_TOWLOWER
/*======================================
 * wz_makelower -- widechar lowercasing
 * Input/output holds wchar_t characters
 *====================================*/
static void
wz_makelower (ZSTR zstr)
{
	wchar_t * wp;
	for (wp = (wchar_t *)zs_str(zstr); *wp; ++wp) {
		if (*wp == 0x3A3) {
			/* Greek capital sigma */
			if (!iswletter(wp[1]))
				*wp = 0x3C2; /* final lower sigma */
			else
				*wp = 0x3C3; /* medial lower sigma */
		} else {
			*wp = towlower(*wp);
		}
	}
}
#endif
#ifdef HAVE_TOWUPPER
/*======================================
 * wz_makeupper -- widechar uppercasing
 * Input/output holds wchar_t characters
 *====================================*/
static void
wz_makeupper (ZSTR zstr)
{
	/* TODO: preprocess for the German special case */
	wchar_t * wp;
	for (wp = (wchar_t *)zs_str(zstr); *wp; ++wp) {
		*wp = towupper(*wp);
	}
}
#endif
/*======================================
 * wz_makenarrow -- convert widechar to UTF-8
 * Done inplace
 *====================================*/
static void
wz_makenarrow(ZSTR zstr)
{
	ZSTR zout = makeznarrow(zstr);
	zs_move(zstr, &zout);
}
/*======================================
 * ll_tolowerz -- Return lowercase version of string
 *====================================*/
ZSTR
ll_tolowerz (CString s, int utf8)
{
	ZSTR zstr=0;
	if (utf8) {
		if (lowerfunc)
			return (*lowerfunc)(s);
#ifdef HAVE_TOWLOWER
		zstr = makewide(s);
		if (zstr) {
			/* Now zstr holds a string of wchar_t characters */
			/* NB: sizeof(wchar_t) varies with platform */
			wz_makelower(zstr);
			wz_makenarrow(zstr);
		}
#endif
	}

	if (!zstr) {
		zstr = zs_newn(strlen(s));
		for ( ; *s; ++s) {
			u_char ch = (u_char)*s;
			/* uppercase unless UTF-8 multibyte code */
			if (!utf8 || ch < 0x7f)
				ch = ll_tolower(ch);
			zs_appc(zstr, ch);
		}
	}
	return zstr;
}
/*==========================================
 * set_utf8_casing -- Set plugin functions for UTF-8 casing
 *========================================*/
void
set_utf8_casing (ZSTR (*ufnc)(CString), ZSTR (*lfnc)(CString))
{
	upperfunc = ufnc;
	lowerfunc = lfnc;
}
/*==========================================
 * ll_toupperz -- Return uppercase version of string
 *========================================*/
ZSTR
ll_toupperz (CString s, int utf8)
{
	ZSTR zstr=0;
	if (utf8) {
		if (upperfunc)
			return (*upperfunc)(s);
#ifdef HAVE_TOWUPPER
		/* This should not normally be used.
		There should be a plugin (upperfunc) */
		zstr = makewide(s);
		if (zstr) {
			/* Now zstr holds a string of wchar_t characters */
			/* NB: sizeof(wchar_t) varies with platform */
			wz_makeupper(zstr);
			wz_makenarrow(zstr);
		}
#endif
	}

	if (!zstr) {
		zstr = zs_newn(strlen(s));
		for ( ; *s; ++s) {
			u_char ch = (u_char)*s;
			/* uppercase unless UTF-8 multibyte code */
			if (!utf8 || ch < 0x7f)
				ch = ll_toupper(ch);
			zs_appc(zstr, ch);
		}
	}
	return zstr;
}
/*======================================
 * upperascii_s -- Convert string to uppercase
 * This only handle ASCII letters
 *  returns static buffer
 * (It is a fast, cheap solution appropriate
 *  for GEDCOM keyword parsing.)
 *====================================*/
String
upperascii_s (String str)
{
	static char scratch[MAXLINELEN+1];
	String p = scratch;
	int c, i=0;
	while ((c = (u_char)*str++) && (++i < MAXLINELEN+1)) {
		if (c>='a' && c<='z')
			c += 'A' - 'a';
		*p++ = (unsigned int)c;
	}
	*p = '\0';
	return scratch;
}
/*================================
 * ll_tocapitalizedz -- Returned capitalized version of string
 *==============================*/
ZSTR
ll_tocapitalizedz (String s, int utf8)
{
	ZSTR zstr=0;
#if defined(HAVE_TOWLOWER) && defined(HAVE_TOWUPPER)
	if (utf8) {
		zstr = makewide(s);
		if (zstr) {
			wchar_t * wp;
			/* Now zstr holds a string of wchar_t characters */
			/* NB: sizeof(wchar_t) varies with platform */
			/* capitalize first letter */
			wp = (wchar_t *)zs_str(zstr);
			*wp = towupper(*wp);
			wz_makenarrow(zstr);
		}
	}
#endif

	if (!zstr) {
		zstr = zs_newn(strlen(s));
		for ( ; *s; ++s) {
			u_char ch = (u_char)*s;
			if (!zs_len(zstr)) {
				/* uppercase unless UTF-8 multibyte code */
				if (!utf8 || ch < 0x7f)
					ch = ll_toupper(ch);
			}
			zs_appc(zstr, ch);
		}
	}
	return zstr;
}
/*================================
 * ll_totitlecasez -- Return titlecased version of string
 *==============================*/
ZSTR
ll_totitlecasez (String s, int utf8)
{
	ZSTR zstr=0;
#if defined(HAVE_TOWLOWER) && defined(HAVE_TOWUPPER) && defined(HAVE_ISWSPACE)
	if (utf8) {
		zstr = makewide(s);
		if (zstr) {
			/* Now zstr holds a string of wchar_t characters */
			/* NB: sizeof(wchar_t) varies with platform */
			bool inword = false;
			wchar_t * wp;
			for (wp = (wchar_t *)zs_str(zstr); *wp; ++wp) {
				if (iswspace(*wp)) {
					inword = false;
				} else {
					if (!inword) {
						/* first letter of word */
						*wp = towupper(*wp);
						inword = true;
					}
				}
			}
			wz_makenarrow(zstr);
		}
	}
#endif

	if (!zstr) {
		bool inword = false;
		zstr = zs_newn(strlen(s));
		for ( ; *s; ++s) {
			u_char ch = (u_char)*s;
			if (iswhite((u_char)*s)) {
				inword = false;
			} else if (!inword) {
				/* first letter of word */
				inword = true;
				/* uppercase unless UTF-8 multibyte code */
				if (!utf8 || ch < 0x7f)
					ch = ll_toupper(*s);
			}
			zs_appc(zstr, ch);
		}
	}
	return zstr;
}

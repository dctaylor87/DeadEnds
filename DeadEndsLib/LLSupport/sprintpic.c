/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * sprintpic.c -- snprintf versions which handle reordered arguments
 *  (because not everyone is fortunate enough to have glibc)
 *==============================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>

#include "standard.h"
#include "denls.h"

#include "zstr.h"
#include "de-strings.h"
#include "sprintpic.h"

/*********************************************
 * local function prototypes
 *********************************************/

/* alphabetical */
static bool printpic_arg(String *b, int max, int utf8, CString arg, int arglen);

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*==============================
 * printpic_arg -- Print an arg to a string
 *  This is the heart of sprintpic, here so we
 *  can use it in all sprintpic's.
 *  b:      [I/O] pointer to output buffer
 *  max:    [IN]  space left in output buffer
 *  arg:    [IN]  arg to insert
 *  arglen: [IN]  (precomputed) length of arg
 * returns false if can't fit entire arg.
 *============================*/
static bool
printpic_arg (String *b, int max, int utf8, CString arg, int arglen)
{
	if (!arglen) return true;
	if (arglen > max) {
		/* can't fit it all */
		destrncpy(*b, arg, max+1, utf8); 
		b[max] = 0;
		return false;
	} else {
		/* it fits */
		strcpy(*b, arg);
		*b += arglen;
		return true;
	}
}
/*=========================================
 * sprintpic0 -- Print using a picture string
 *  with no arguments
 * This is just snprintf, but fixed to always
 * zero-terminate.
 *=======================================*/
void
sprintpic0 (String buffer, int len, int utf8, CString pic)
{
	if (len == snprintf(buffer, len, "%s", pic)) {
		/* overflowed -- back up to last character that fits */
		int width=0;
		String prev = find_prev_char(&buffer[len-1], &width, buffer, utf8);
		prev[width]=0;
	}

}
/*==============================
 * sprintpic1 -- Print using a picture string
 *  with one argument, eg "From %1"
 *  buffer:  [I/O] output buffer
 *  len:     [IN]  size of output buffer
 *  pic:     [IN]  picture string
 *  arg1:    [IN]  argument (for %1)
 * (Multiple occurrences of %1 are allowed.)
 * returns false if it couldn't fit whole thing.
 *============================*/
bool
sprintpic1 (String buffer, int len, int utf8, CString pic, CString arg1)
{
	String b = buffer, bmax = &buffer[len-1];
	CString p=pic;
	int arg1len = arg1 ? strlen(arg1) : 0; /* precompute */
	while (1) {
		if (p[0]=='%' && p[1]=='1') {
			if (!printpic_arg(&b, bmax-b, utf8, arg1, arg1len))
				return false;
			p += 2;
		} else {
			*b++ = *p++;
		}
		if (!p[0]) { /* ran out of input */
			*b=0;
			return true;
		}
		if (b == bmax) { /* ran out of output room */
			*b=0;
			return false;
		}
	}
}
/*==============================
 * zprintpic1 -- Print using a picture string
 *  with one argument, eg "From %1"
 *  pic:     [IN]  picture string
 *  arg1:    [IN]  argument (for %1)
 * (Multiple occurrences of %1 are allowed.)
 * returns result as zstring
 *============================*/
ZSTR
zprintpic1 (CString pic, CString arg1)
{
	ZSTR zstr = zs_newn(strlen(pic)+strlen(arg1));
	CString p=pic;
	while (*p) {
		if (p[0]=='%' && p[1]=='1') {
			zs_apps(zstr, arg1);
			p += 2;
		} else {
			zs_appc(zstr, *p++);
		}
	}
	return zstr;
}
/*==============================
 * sprintpic2 -- Print using a picture string
 *  with two arguments, eg "From %1 To %s"
 * See sprintpic1 for argument explanation.
 *============================*/
bool
sprintpic2 (String buffer, int len, int utf8, CString pic, CString arg1, CString arg2)
{
	String b = buffer, bmax = &buffer[len-1];
	CString p=pic;
	int arg1len = arg1 ? strlen(arg1) : 0; /* precompute */
	int arg2len = arg2 ? strlen(arg2) : 0;
	while (1) {
		if (p[0]=='%' && p[1]=='1') {
			if (!printpic_arg(&b, bmax-b, utf8, arg1, arg1len))
				return false;
			p += 2;
		} else if (p[0]=='%' && p[1]=='2') {
			if (!printpic_arg(&b, bmax-b, utf8, arg2, arg2len))
				return false;
			p += 2;
		} else {
			*b++ = *p++;
		}
		if (!p[0]) { /* ran out of input */
			*b=0;
			return true;
		}
		if (b == bmax) { /* ran out of output room */
			*b=0;
			return false;
		}
	}
}
/*==============================
 * zprintpic2 -- Print using a picture string
 *  with two arguments, eg "From %1 To %s"
 * returning result as zstring
 *============================*/
ZSTR
zprintpic2 (CString pic, CString arg1, CString arg2)
{
	ZSTR zstr = zs_newn(strlen(pic)+strlen(arg1)+strlen(arg2));
	CString p=pic;
	while (*p) {
		if (p[0]=='%' && p[1]=='1') {
			zs_apps(zstr, arg1);
			p += 2;
		} else if (p[0]=='%' && p[1]=='2') {
			zs_apps(zstr, arg2);
			p += 2;
		} else {
			zs_appc(zstr, *p++);
		}
	}
	return zstr;
}
/*==============================
 * sprintpic3 -- Print using a picture string
 *  with three arguments, eg "%1/%2/%3"
 * See sprintpic1 for argument explanation.
 *============================*/
bool
sprintpic3 (String buffer, int len, int utf8, CString pic, CString arg1, CString arg2
	, CString arg3)
{
	String b = buffer, bmax = &buffer[len-1];
	CString p=pic;
	int arg1len = arg1 ? strlen(arg1) : 0; /* precompute */
	int arg2len = arg2 ? strlen(arg2) : 0;
	int arg3len = arg3 ? strlen(arg3) : 0;
	while (1) {
		if (p[0]=='%' && p[1]=='1') {
			if (!printpic_arg(&b, bmax-b, utf8, arg1, arg1len))
				return false;
			p += 2;
		} else if (p[0]=='%' && p[1]=='2') {
			if (!printpic_arg(&b, bmax-b, utf8, arg2, arg2len))
				return false;
			p += 2;
		} else if (p[0]=='%' && p[1]=='3') {
			if (!printpic_arg(&b, bmax-b, utf8, arg3, arg3len))
				return false;
			p += 2;
		} else {
			*b++ = *p++;
		}
		if (!p[0]) { /* ran out of input */
			*b=0;
			return true;
		}
		if (b == bmax) { /* ran out of output room */
			*b=0;
			return false;
		}
	}
}
/*==============================
 * zprintpic3 -- Print using a picture string
 *  with three arguments, eg "%1/%2/%3"
 * returning result as zstring
 *============================*/
ZSTR
zprintpic3 (CString pic, CString arg1, CString arg2, CString arg3)
{
	ZSTR zstr = zs_newn(strlen(pic)+strlen(arg1)+strlen(arg2)+strlen(arg3));
	CString p=pic;
	while (*p) {
		if (p[0]=='%' && p[1]=='1') {
			zs_apps(zstr, arg1);
			p += 2;
		} else if (p[0]=='%' && p[1]=='2') {
			zs_apps(zstr, arg2);
			p += 2;
		} else if (p[0]=='%' && p[1]=='3') {
			zs_apps(zstr, arg3);
			p += 2;
		} else {
			zs_appc(zstr, *p++);
		}
	}
	return zstr;
}

/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * strutf.c -- UTF-8 specific string functions
 *==============================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(DEADENDS)
#include <ansidecl.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "llnls.h"

#include "de-strings.h"

#else

#include "llstdlib.h"

#endif
/*==============================
 * utf8len -- Length of a UTF-8 character
 *  ch: [in] starting byte
 * Created: 2001/08/02 (Perry Rapp)
 *============================*/
#if 0
int
utf8len (char ch)
{
	/* test short cases first, as probably much more common */
	if (!((ch & 0x80) && (ch & 0x40)))
		return 1; /* not a multibyte lead byte */
	if (!(ch & 0x20))
		return 2;
	if (!(ch & 0x10))
		return 3;
	if (!(ch & 0x08))
		return 4;
	if (!(ch & 0x04))
		return 5;
	return 6;
}
#endif

int
utf8len (char ch)
{
  /* verify byte is first byte of a character */
  if ((ch & 0xc0) == 0x80)
    {
      /* byte starts with 10 -- it's not the starting byte! */

      /* XXX insert code to generate an error or print an error
	 message -- byte is not the first byte of a character XXX */
      return 0;
    }

  if ((ch & 0x80) == 0)
    return 1;			/* top bit clear -- single byte */
  else if ((ch & 0xe0) == 0xc0)
    return 2;			/* top 3 bits are 110 */
  else if ((ch & 0xf0) == 0xe0)
    return 3;			/* top 4 bits are 1110 */
  else if ((ch & 0xf8) == 0xf0)
    return 4;			/* top 5 bits are 1111 0 */

#if 1
  /* to support the no longer supported 5 and 6 byte characters */
  else if ((ch & 0xfc) == 0xf8)
    return 5;			/* top 6 bits are 1111 10 */
  else if ((ch & 0xfe) == 0xfc)
    return 6;			/* top 7 bits are 1111 110 */

  /* to continue the pattern and support the never supported 7 and 8
     byte characters... */
  else if ((ch & 0xff) == 0xfe)
    return 7;
  else
    return 8;
#else
  /* XXX insert code to generate an error or print an error
     message -- it is not a valid character encoding XXX */
  return 0;
#endif
}
/*==============================
 * str8chlen -- Length of a UTF-8 string in characters
 * Created: 2001/08/02 (Perry Rapp)
 *============================*/
size_t
str8chlen (CString str)
{
	CString ptr = str;
	size_t len = 0;
	size_t chwidth = 0;
	size_t i=0;
	while (*ptr) {
		chwidth = utf8len(*ptr);
		++len;
		/* advance pointer, but be careful
		not to skip blindly over null in middle
		of broken multibyte sequence */
		for (i=0; i<chwidth; ++i) {
			if (!ptr[0])
				return len;
			++ptr;
		}
	}
	return len;
}
/*=========================================
 * find_prev_char -- Back up to start of previous character.
 * Return pointer to start of previous character,
 *  and optionally its width.
 * (This is of course trivial if we're not in UTF-8 mode.)
 * If limit is non-zero, don't back up beyond this.
 * This will set *width=0 if it gets back to limit without 
 *  finding valid character, or backs up more than 6 characters
 * Warning: return width + pointer may exceed lenght of string
 *  if input was pointing at partial multibyte at end (broken string)
 * Created: 2002/06/12, Perry Rapp
 *=======================================*/
String
find_prev_char (String ptr, int * width, String limit, int utf8)
{
	String init = ptr;
	int len=0;
	if (utf8) {
		while (1) {
			int tmp;
			if (ptr == limit)
				break;
			--ptr;
			if (init - ptr == 7)
				break;
			/* if UTF-8 trail byte, keep backing up */
			if ((*ptr & 0x80) && !(*ptr & 0x40))
				continue;
			tmp = utf8len(*ptr);
			len = tmp;
			break;
		}
	} else {
		if (ptr != limit) {
			len = 1;
			--ptr;
		}
	}
	if (width)
		*width = len;
	return ptr;
}
/*=========================================
 * next_char32 -- return char and advance pointer
 *  This is trivial unless we're in UTF-8
 * character returned is in UCS-4, native alignment
 * Assumes valid UTF-8 input
 *=======================================*/
int
next_char32 (String * ptr, int utf8)
{
	String str = *ptr; /* simplifies notation below */
	if (!utf8) {
		int ch = (unsigned char)*str;
		++(*ptr);
		return ch;
	}
	/* test short cases first, as probably much more common */
	if (!(*str & 0x80 && *str & 0x40)) {
		/* if a trail byte, comes thru here & need cast */
		int ch = (unsigned char)str[0];
		++(*ptr);
		return ch;
	}
	if (!(*str & 0x20)) {
		int ch = ((str[0] & 0x1F) << 6)
			+ (str[1] & 0x3F);
		(*ptr) += 2;
		return ch;
	}
	if (!(*str & 0x10)) {
		int ch = ((str[0] & 0x0f) << 12)
			+ ((str[1] & 0x3F) << 6)
			+ (str[2] & 0x3F);
		(*ptr) += 3;
		return ch;
	}
	if (!(*str & 0x08)) {
		int ch = ((str[0] & 0x0F) << 18)
			+ ((str[1] & 0x3F) << 12)
			+ ((str[2] & 0x3F) << 6)
			+ (str[3] & 0x3F);
		(*ptr) += 4;
		return ch;
	}
	if (!(*str & 0x04)) {
		int ch = ((str[0] & 0x0F) << 24)
			+ ((str[1] & 0x3F) << 18)
			+ ((str[2] & 0x3F) << 12)
			+ ((str[3] & 0x3F) << 6)
			+ (str[4] & 0x3F);
		(*ptr) += 5;
		return ch;
	} else {
		int ch = ((str[0] & 0x0F) << 30)
			+ ((str[1] & 0x3F) << 24)
			+ ((str[2] & 0x3F) << 18)
			+ ((str[3] & 0x3F) << 12)
			+ ((str[4] & 0x3F) << 6)
			+ (str[5] & 0x3F);
		(*ptr) += 6;
		return ch;
	}
}
/*=========================================
 * skip_BOM -- advance string past BOM if present
 *  BOM is a byte order mark that Microsoft likes to use
 *=======================================*/
void
skip_BOM (String * pstr)
{
	String str = *pstr;
	/* UTF-8 is the only BOM we handle */
	if ((u_char)str[0] == 0xEF && (u_char)str[1] == 0xBB && (u_char)str[2] == 0xBF)
		*pstr += 3;
}
/*=========================================
 * unicode_to_utf8 -- convert UCS-4 (native alignment) to UTF-8
 *=======================================*/
void
unicode_to_utf8 (int wch, char * utf8)
{
	unsigned char *lpd = (unsigned char *)utf8;
	unsigned int uch = (unsigned int)wch;
	if (uch < 0x80)
	{
		*lpd++ = uch;
	}
	else if (uch < 0x800)
	{
		*lpd++ = 0xC0 + (uch >> 6);
		*lpd++ = 0x80 + (uch & 0x3F);
	}
	else if (uch < 0x10000)
	{
		*lpd++ = 0xE0 + (uch >> 12);
		*lpd++ = 0x80 + ((uch >> 6) & 0x3F);
		*lpd++ = 0x80 + (uch & 0x3F);
	}
	else if (uch < 0x200000)
	{
		*lpd++ = 0xF0 + (uch >> 18);
		*lpd++ = 0x80 + ((uch >> 12) & 0x3F);
		*lpd++ = 0x80 + ((uch >> 6) & 0x3F);
		*lpd++ = 0x80 + (uch & 0x3F);
	}
	else if (uch < 0x4000000)
	{
		*lpd++ = 0xF8 + (uch >> 24);
		*lpd++ = 0x80 + ((uch >> 18) & 0x3F);
		*lpd++ = 0x80 + ((uch >> 12) & 0x3F);
		*lpd++ = 0x80 + ((uch >> 6) & 0x3F);
		*lpd++ = 0x80 + (uch & 0x3F);
	}
	else if (uch < 0x80000000)
	{
		*lpd++ = 0xF8 + (uch >> 30);
		*lpd++ = 0x80 + ((uch >> 24) & 0x3F);
		*lpd++ = 0x80 + ((uch >> 18) & 0x3F);
		*lpd++ = 0x80 + ((uch >> 12) & 0x3F);
		*lpd++ = 0x80 + ((uch >> 6) & 0x3F);
		*lpd++ = 0x80 + (uch & 0x3F);
	}
	else
	{
		*lpd++ = '?';
	}
	*lpd++ = 0;
}
/*=========================================
 * chopstr_utf8 -- chop string at specified byte index
 * Eg, chopstr_utf8("abc", 2, x) results in "ab"
 * If UTF-8, backs up as necessary to avoid leaving
 * broken multibyte at end
 *=======================================*/
void
chopstr_utf8 (String str, size_t index, bool utf8)
{
	int width=0;
	String prev = find_prev_char(&str[index+1], &width, str, utf8);
	prev[0] = 0;
	/* We don't zero out entire width in case it is outside of
	string range, in case caller passed us a broken string */
}
/*=========================================
 * limit_width -- chop string if longer than specified width
 *  handles UTF-8
 *=======================================*/
void
limit_width (String str, size_t width, bool utf8)
{
	if (strlen(str) <= width) return;
	chopstr_utf8(str, width-1, utf8);
}


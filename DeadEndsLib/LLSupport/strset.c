/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * strset.c -- string assignment functions
 *  These provide a consistent API mirroring the strapp API
 *==============================================================*/


#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>

#include "standard.h"
#include "llnls.h"

#include "de-strings.h"

/*==================================
 * destrset -- destrncpy adopted to strset/strapp API
 * handles UTF-8
 *================================*/
char *
destrsets (char *dest, size_t limit, int utf8, const char *src)
{
	return destrncpy(dest, src, limit, utf8);
}
/*==================================
 * destrsetc -- destrset with a character argument
 *================================*/
char *
destrsetc (char *dest, size_t limit, char ch)
{
	int utf8 = 0;
	char src[2];
	src[0] = ch;
	src[1] = 0;
	return destrsets(dest, limit, utf8, src);
}
/*==================================
 * destrsetf -- snprintf style assignment to string,
 * subject to length limit (including current contents)
 * handles UTF-8
 *================================*/
char *
destrsetf (char * dest, int limit, int utf8, const char * fmt, ...)
{
	va_list args;
	va_start(args, fmt);
	destrsetvf(dest, limit, utf8, fmt, args);
	va_end(args);
	return dest;
}
/*==================================
 * destrsetvf -- vsnprintf style assignment to string,
 * subject to length limit (including current contents)
 * handles UTF-8
 *================================*/
char *
destrsetvf (char * dest, int limit, int utf8, const char * fmt, va_list args)
{
	return destrncpyvf(dest, limit, utf8, fmt, args);
}



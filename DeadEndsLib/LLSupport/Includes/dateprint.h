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
 * date.h - Header file for date.c
 * Copyright(c) 1991-95 by T.T. Wetmore IV; all rights reserved
 *===========================================================*/

#ifndef _DATE_H
#define _DATE_H

enum { BAD_YEAR=-99999 };
struct tag_dnum {
	int val;    /* parsed value, eg, 2 for FEBRUARY */
	int val2;   /* TODO: need comment explaining what this is */
	String str; /* raw string that was in date */
};

/* complex picture strings */
enum { ECMPLX_ABT, ECMPLX_EST, ECMPLX_CAL, ECMPLX_BEF, ECMPLX_AFT
	, ECMPLX_BET_AND, ECMPLX_FROM, ECMPLX_TO, ECMPLX_FROM_TO
	, ECMPLX_END };

typedef struct tag_gdateval *GDATEVAL; /* definition of struct in datei.h */

ZSTR approx_time(int seconds);
GDATEVAL create_gdateval(void);
String do_format_date(String, int, int, int, int, int, int);
void date_update_lang(void);
int date_get_day(GDATEVAL gdv);
int date_get_mod(GDATEVAL gdv);
int date_get_month(GDATEVAL gdv);
int date_get_year(GDATEVAL gdv);
String date_get_year_string(GDATEVAL gdv);
GDATEVAL extract_date(String);
void free_gdateval(GDATEVAL gdv);
bool gdateval_isdual(GDATEVAL);
String get_todays_date(void);
bool is_valid_dayfmt(int dayfmt);
bool is_valid_monthfmt(int monthfmt);
bool is_valid_yearfmt(int yearfmt);
bool set_cmplx_pic(int ecmplx, String pic);
void set_date_pic(String pic);
String shorten_date(String);
void term_date(void);



#endif /* _DATE_H */

/* 
   Copyright (c) 2002-2007 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef codesets_h_included
#define codesets_h_included


extern String int_codeset;     /* internal codeset */
#if defined(DEADENDS)
extern bool uu8;	       /* is int_codeset utf-8? */
extern bool gui8;	       /* flag if display output encoding is UTF-8 */
#endif

extern String editor_codeset_out; /* output to editor */
extern String editor_codeset_in;  /* input from editor */
extern String gedcom_codeset_out; /* output GEDCOM files */
extern String gedcom_codeset_in;  /* default for reading GEDCOM files */
extern String gui_codeset_in;     /* reading characters from GUI */
extern String gui_codeset_out;    /* writing characters to GUI */
extern String report_codeset_out; /* default for report output */
extern String report_codeset_in;  /* default for input from reports */

CString get_defcodeset(void);
void init_codesets(void);
void term_codesets(void);
bool is_codeset_utf8(CString codename);

#endif /* codesets_h_included */


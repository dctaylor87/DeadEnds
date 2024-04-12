/* 
   Copyright (c) 2000-2002 Perry Rapp
   Copyright (c) 2003 Matt Emmerton
   "The MIT license"

   Permission is hereby granted, free of charge, to any person
   obtaining a copy of this software and associated documentation
   files (the "Software"), to deal in the Software without
   restriction, including without limitation the rights to use, copy,
   modify, merge, publish, distribute, sublicense, and/or sell copies
   of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
   AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
   DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * ui.h -- UI function prototypes
 * Copyright(c) 2002 by Perry Rapp; all rights reserved
 * Copyright(c) 2003 by Matt Emmerton; all rights reserved
 *===========================================================*/

/* Prototypes */
int ask_for_char(CString ttl, CString prmpt, CString ptrn);
int ask_for_char_msg(CString msg, CString ttl, CString prmpt, CString ptrn);
bool ask_for_db_filename (CString ttl, CString prmpt, CString basedir, String buffer, int buflen);
bool ask_for_filename_impl(String ttl, String path, String prmpt, String buffer, int buflen);
bool ask_for_program (String mode, String ttl, String *pfname, String *pfullpath, String path, String ext, bool picklist);
int choose_from_array (String ttl, int no, String *pstrngs);
int choose_from_list (String ttl, List *list);
int choose_list_from_indiseq (String ttl, Sequence *seq);
int choose_one_from_indiseq (String ttl, Sequence *seq);
int choose_one_or_list_from_indiseq(String ttl, Sequence *seq, bool multi); /* XXX */
int choose_or_view_array(String ttl, int no, String *pstrngs, bool selectable);
int prompt_stdout (String prompt);
void view_array (String ttl, int no, String *pstrngs);
bool yes_no_value(int c);
Sequence *invoke_search_menu (void);


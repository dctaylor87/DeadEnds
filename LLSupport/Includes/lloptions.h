/* 
   lloptions.h
   Copyright (c) 2000-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*
 lloptions.h - handling for (opaque) string & numeric options

  Options are fetched on-the-fly when requested
   first from report properties (if getdeoptstr_rpt or getoptint_rpt)
   then from db properties
   then from global properties
   then from fallback properties
*/

#ifndef LLOPTIONS_H_INCLUDED
#define LLOPTIONS_H_INCLUDED

/* initialization & termination */
void free_optable(HashTable **ptab);
//void get_db_options(HashTable *opts); /* free with FREEBOTH */
int load_global_options(String configfile, String * pmsg);
void register_notify(CALLBACK_FNC fncptr);
void set_cmd_options(HashTable *opts);
//void set_db_options(HashTable *opts);
void setoptstr_fallback(String optname, String newval);
void term_lloptions(void);
void unregister_notify(CALLBACK_FNC fncptr);


/* routine use */
/* TODO: fix const-correctness */
String getdeoptstr(CString optname, String defval);
int getdeoptint(CString optname, int defval);
String getdeoptstr_dbonly(CString optname, String defval);
String getdeoptstr_rpt(CString optname, String defval);

extern bool is_dir_sep (char c);

#endif /* OPTIONS_H_INCLUDED */

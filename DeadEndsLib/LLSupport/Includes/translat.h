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
/*===========================================================
 * translat.h -- Header file for translate feature
 * Copyright(c) 1994 by T.T. Wetmore IV; all rights reserved
 * pre-SourceForge version information:
 *   3.0.0 - 29 May 1994
 *=========================================================*/

#ifndef _TRANSLAT_H
#define _TRANSLAT_H

/* Types */

/*
Need translation chain
 made up of translation step
 each step is either iconv step or custom step
 need routine to build chain (check for rule, & try to construct chain)
 put embedded table into chain (?) with flag for which end, or put into list ? how does this work now when edited ?
*/

/* A TRANTABLE is a single conversion table; multiple 
TRANTABLES may make up one xlat */
typedef struct tag_trantable *TRANTABLE;

/* An XLAT is a conversion system between two codesets; it may 
contain TRANTABLES and iconv steps, and even a legacy tt */
typedef struct tag_xlat *XLAT;


/* Functions */

CString transl_get_map_name(int trnum);
TRANTABLE create_trantable(String *lefts, String *rights, int n, String name);
bool init_map_from_rec(CString key, int trnum, TRANTABLE*);
//bool custom_sort(const char *str1, const char *str2, int * rtn);
ZSTR get_trantable_desc(TRANTABLE tt);
void remove_trantable(TRANTABLE);
void translate_catn(XLAT ttm, String * pdest, CString src, int * len);
void translate_string(XLAT, CString in, String out, int max);
ZSTR translate_string_to_zstring(XLAT ttm, CString in);
bool translate_write(XLAT ttm, String in, int *lenp, FILE *ofp, bool last);
CString tt_get_name(TRANTABLE tt);


/*
New system under development 2002-11-25+
translat is the frontend, which knows about the various codesets (internal, GUI, ...)
 translat uses codesets & xlat
 xlat is the translation system
   xlat uses charmaps
*/
bool transl_are_all_conversions_ok(void);
void transl_free_predefined_xlats(void);
XLAT transl_get_predefined_xlat(int trnum);
ZSTR transl_get_predefined_menukey(int trnum);
ZSTR transl_get_predefined_name(int trnum);
ZSTR transl_get_description(XLAT xlat);
XLAT transl_get_xlat(CString src, CString dest);
XLAT transl_get_xlat_to_int(CString codeset);
bool transl_is_xlat_valid(XLAT xlat);
TRANTABLE transl_get_legacy_tt(int trnum);
void transl_load_all_tts(void);
void transl_load_xlats(void);
void transl_parse_codeset(CString codeset, ZSTR zcsname, List **subcodes);
void transl_release_xlat(XLAT xlat);
void transl_set_legacy_tt(int trnum, TRANTABLE tt);
void transl_xlat(XLAT xlat, ZSTR zstr);

#endif /* _TRANSLAT_H */

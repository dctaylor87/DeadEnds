/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * proptbls.h -- functions for making lists or arrays of property tables
 *  for files (ie, a property table for each file)
 *==============================================================*/

#ifndef proptbls_h_included
#define proptbls_h_included

/* proptbls.c */
typedef int (*SELECT_FNC)(const struct dirent *);
int add_dir_files_to_proplist(CString dir, SELECT_FNC selectfnc, List *list);
int add_path_files_to_proplist(CString path, SELECT_FNC selectfnc, List *list);
void add_prop_dnum(HashTable *props, CString name, CString value);
HashTable ** convert_proplist_to_proparray(List *list);
void free_proparray(HashTable *** props);
HashTable ** get_proparray_of_files_in_path(CString path, SELECT_FNC selectfnc, int * nfiles);
void set_prop_dnum(HashTable *props, int n, CString name, CString value);

#endif /* proptbls_h_included */

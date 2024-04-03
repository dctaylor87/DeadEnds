/* 
   Copyright (c) 2001-2002 Perry Rapp
   "The MIT license"
   Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:
   The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
/*=============================================================
 * proptbls.c -- functions for making lists or arrays of property tables
 *  for files (ie, a property table for each file)
 *==============================================================*/

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#if defined(DEADENDS)
#include <ansidecl.h>
#include <inttypes.h>
#include <dirent.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "llnls.h"
#include "sys_inc.h"

#include "list.h"
#include "hashtable.h"
#include "stringtable.h"
#include "refnindex.h"
#include "gnode.h"
#include "database.h"
#include "readwrite.h"
#include "de-strings.h"
#include "proptbls.h"

#else
#include "llstdlib.h"
#include "arch.h" /* dirent used in scandir */
#include "table.h"
#include "proptbls.h"

#endif

/*==========================================================
 * add_prop_dnum -- Add named property table as new last display order
 *========================================================*/
void
add_prop_dnum (HashTable *props, CString name, CString value)
{
	String str = searchStringTable(props, "dn");
	int n = ll_atoi(str, 0)+1;
	char temp[FMT_INT_LEN+1];
	snprintf(temp, sizeof(temp), "d" FMT_INT, n);
	insertInStringTable(props, temp, name);
	insertInStringTable(props, name, value);
	snprintf(temp, sizeof(temp), FMT_INT, n);
	insertInStringTable(props, "dn", temp);
}
/*==========================================================
 * set_prop_dnum -- Set named property in table, at specified display number
 *========================================================*/
void
set_prop_dnum (HashTable *props, int n, CString name, CString value)
{
	String str = searchStringTable(props, "dn");
	int max = ll_atoi(str, 0);
	char temp[24];
	snprintf(temp, sizeof(temp), "d" FMT_INT, n);
	insertInStringTable(props, temp, name);
	insertInStringTable(props, name, value);
	if (n>max) {
		snprintf(temp, sizeof(temp), FMT_INT, n);
		insertInStringTable(props, "dn", temp);
	}
}
/*===================================================
 * add_dir_files_to_proplist -- Add all files in dir to list of property tables
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
int
add_dir_files_to_proplist (CString dir, SELECT_FNC selectfnc, List *list)
{
	struct dirent **programs;
	int n = scandir(dir, &programs, selectfnc, alphasort);
	int i;
	for (i=0; i<n; ++i) {
		HashTable *table = createStringTable();
		set_prop_dnum(table, 1, "filename", programs[i]->d_name);
		set_prop_dnum(table, 2, "dir", dir);
		stdfree(programs[i]);
		programs[i] = NULL;
		enqueueList(list, table);
	}
	if (n>0)
		stdfree(programs);
	return i;
}
/*===================================================
 * add_path_files_to_proplist -- Add all files on path to list of property tables
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
int
add_path_files_to_proplist (CString path, SELECT_FNC selectfnc, List *list)
{
	String dirs, p;
	int ct=0;
	if (!path || !path[0]) return 0;
	dirs = (String)stdalloc(strlen(path)+2);
	chop_path(path, dirs);
	for (p=dirs; *p; p+=strlen(p)+1) {
		add_dir_files_to_proplist(p, selectfnc, list);
	}
	return ct;
}
/*===================================================
 * convert_proplist_to_proparray -- 
 *  Convert a list of property tables to an array of same
 *  Consumes the list (actually removes it at end)
 *  Output array is one larger than list, and last entry is NULL
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
HashTable **
convert_proplist_to_proparray (List *list)
{
	HashTable ** props;
	int i;
	props = (HashTable **)malloc((lengthList(list)+1)*sizeof(props[0]));
	i = 0;
	FORLIST(list, el)
		props[i++] = (TABLE)el;
	ENDLIST
	props[i] = NULL; /* null marker at end of array */
	deleteList(list);
	return props;
}
/*===================================================
 * get_proparray_of_files_in_path -- get array of property tables of files in path
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
HashTable **
get_proparray_of_files_in_path (CString path, SELECT_FNC selectfnc, int * nfiles)
{
	/* get array of file property tables */
	List *list = create_list();
	add_path_files_to_proplist(path, selectfnc, list);
	*nfiles = lengthList(list);
	return convert_proplist_to_proparray(list);
}
/*===================================================
 * free_proparray -- free array of property tables
 *  NB: must have NULL marker entry at end
 * Created: 2002/10/19, Perry Rapp
 *=================================================*/
void
free_proparray (HashTable *** props)
{
	int i;
	for (i=0; (*props)[i]; ++i) {
		HashTable *tab = (*props)[i];
		deleteHashTable(tab);
	}
	stdfree((*props));
	*props = NULL;
}

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
/*==============================================================
 * brwslist.c -- Browse list operations
 * Copyright (c) 1993-94 by T.T. Wetmore IV; all rights reserved
 *   3.0.2 - 27 Nov 94
 *============================================================*/
#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "standard.h"
#include "denls.h"

#include "hashtable.h"
#include "refnindex.h"
#include "gnode.h"
#include "database.h"
#include "sequence.h"
#include "list.h"
#include "browse.h"

/*********************************************
 * global/exported variables
 *********************************************/

Sequence *current_seq = NULL;

/*********************************************
 * local types
 *********************************************/

struct tag_blel {
	int refcnt; /* ref-countable object */
	String bl_name;
	Sequence *bl_seq;
};
typedef struct tag_blel *BLEL;

/*********************************************
 * local function prototypes
 *********************************************/

static BLEL create_new_blel(void);
/* unused
static void destroy_blel(BLEL blel);
*/

/*********************************************
 * local variables
 *********************************************/

static List *browse_lists=0;

/*********************************************
 * local & exported function definitions
 * body of module
 *********************************************/

/*=====================================================
 *  init_browse_lists -- Initialize named browse lists
 *===================================================*/
void
init_browse_lists (void)
{
  browse_lists = createList(NULL, NULL, NULL, false);
}
/*=====================================================
 *  term_browse_lists -- Termiante named browse lists
 *===================================================*/
void
term_browse_lists (void)
{
	deleteList(browse_lists);
}
/*===========================================
 *  create_new_blel -- Create browse list entry
 *=========================================*/
static BLEL
create_new_blel (void)
{
	BLEL blel = (BLEL) stdalloc(sizeof(*blel));
	memset(blel, 0, sizeof(*blel));
	blel->refcnt = 1;
	return blel;
}
/*===========================================
 *  add_browse_list -- Add named browse list.
 *=========================================*/
void
add_browse_list (String name, Sequence *seq)
{
	BLEL blel;
	bool done = false;
	if (!name) return;
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		if (blel->bl_name && eqstr(name, blel->bl_name)) {
			deleteSequence(blel->bl_seq);
			blel->bl_seq = seq;
			done = true;
			break;
		}
	ENDLIST
	if (done) return;
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		if (!blel->bl_name) {
			blel->bl_name = name;
			blel->bl_seq = seq;
			return;
		}
	ENDLIST
	blel = create_new_blel();
	blel->bl_name = name;
	blel->bl_seq = seq;
	enqueueList(browse_lists, blel);
}
/*=================================================
 *  remove_browse_list -- Remove named browse list.
 *===============================================*/
void
remove_browse_list (String name,
                    Sequence *seq)
{
	BLEL blel;
	deleteSequence(seq);
	if (!name) return;
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		if (blel->bl_name && eqstr(name, blel->bl_name)) {
			blel->bl_name = NULL;
			blel->bl_seq = NULL;
		}
	ENDLIST
}

#if !defined(DEADENDS)		/* DeadEnds now has this in Interp/sequence.c */
/* stringToSequence -- return sequence of records matching a string.
   The search order is: named sequence, key, REFN, name */

Sequence *stringToSequence (CString name, Database *database)
{
  Sequence *seq;

  seq = find_named_seq (name);
  if (! seq)
    seq = keyToSequence (name, database);
  if (! seq)
    seq = refnToSequence (name, database);
  if (! seq)
    seq = nameToSequence (name, database);

  return seq;
}
#endif

/*===========================================
 * find_named_seq -- Find named browse list.
 *=========================================*/
Sequence *
find_named_seq (CString name)
{
	BLEL blel;
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		if (eqstr(name, blel->bl_name)) {
			return copySequence(blel->bl_seq);
		}
	ENDLIST
	return NULL;
}
/*===================================================
 * new_name_browse_list -- Rename named browse list.
 *=================================================*/
void
new_name_browse_list (String oldstr, String newstr)
{
	BLEL blel;
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		if (eqstr(oldstr, blel->bl_name)) {
			stdfree(blel->bl_name);
			blel->bl_name = newstr;
			return;
		}
	ENDLIST
}
/*===================================================
 *  update_browse_list -- Assign name to browse list.
 *=================================================*/
void
update_browse_list (String name,
                    Sequence *seq)
{
	BLEL blel;
	if (!name) {	/* remove anonymous lists */
		deleteSequence(seq);
		return;
	}
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		if (eqstr(name, blel->bl_name))
			blel->bl_seq = seq;
	ENDLIST
}
/*==============================================================
 * remove_from_browse_lists -- Remove stale elements from lists.
 *============================================================*/
void
remove_from_browse_lists (String key)
{
	BLEL blel;
	Sequence *seq;
	if (current_seq) {
		seq = current_seq;
		while (removeFromSequence(seq, key))
			;
	}
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		seq = blel->bl_seq;
		while (removeFromSequence(seq, key))
			;
	ENDLIST
}
/*================================================================
 * rename_from_browse_lists -- Re-figures name of possible element
 *   in browse lists.
 *==============================================================*/
void
rename_from_browse_lists (String key)
{
	Sequence *seq;
	BLEL blel;
	if (current_seq) {
		seq = current_seq;
		renameElementInSequence(seq, key);
	}
	FORLIST(browse_lists, e)
		blel = (BLEL) e;
		seq = blel->bl_seq;
		renameElementInSequence(seq, key);
	ENDLIST
}

/*=================================================
 * destroy_blel -- destroy and free blel (browse list element)
 * All blels are destroyed in this function
 *===============================================*/
/* not used
static void
destroy_blel (BLEL blel)
{
	stdfree(blel->bl_name);
	deleteSequence(blel->bl_seq, false);
	stdfree(blel);
}
*/

/* created by David Taylor */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "standard.h"
#include "denls.h"

#include "hashtable.h"
#include "recordindex.h"
#include "integertable.h"
#include "errors.h"
#include "refnindex.h"
#include "gnode.h"
#include "database.h"
#include "gedcom.h"

#include "refns.h"

/* addRefn -- if refn is already present in the refn index,
                  if it maps to key, returns true
		  if it maps to a different key, returns false
	      otherwise, add the mapping refn --> key to the index,
	      and return true.  */

bool
addRefn (CString refn, CString key, Database *database)
{
  CString refn_key = searchRefnIndex (database->refnIndex, refn);

  if (refn_key)
    {
      /* it already exists -- do the keys match? */
      if (eqstr (refn_key, key))
	return true;
      return false;
    }
  /* it does not already exist */
  addToRefnIndex (database->refnIndex, refn, key);
  database->dirty = true;
  return true;
}

/* removeRefn -- searches refn index, if not found or it maps to a
   different key, then returns false.  Otherwise, removes it and
   returns true. */

bool
removeRefn (CString refn, CString key, Database *database)
{
  CString existing_key = searchRefnIndex (database->refnIndex, refn);

  if (! existing_key || ! eqstr (existing_key, key))
    return false;

  /* we found the REFN and it maps to our key */
  removeFromHashTable (database->refnIndex, refn);

  database->dirty = true;
  return true;
}

/* XXX needs a better name XXX */
/* indexByRefn -- scans all immediate children of node for REFNs.
   For each one found, we add a mapping in the REFN index.

   If any already exist with a different keyt, false (failure) is
   returned indicating one or more failures; otherwise true (success)
   is returned.  All that do not currently exist are added.  */

bool
indexByRefn (GNode *node, Database *database)
{
  bool success = true;
  CString key = node->key;
  CString match;

  for (GNode *new_node = node->child; new_node; new_node = new_node->sibling)
    if (eqstr ("REFN", new_node->tag) && new_node->value)
      /* new_node->value above is paranoia -- REFN nodes are required to have a value */
      {
	CString refn = new_node->value;
	match = searchRefnIndex (database->refnIndex, refn);
	if (match)		/* already present */
	  {
	    if (! eqstr(match, key))
	      success = false;	/* it's not us */
	  }
	else
	  {
	    addToRefnIndex (database->refnIndex, new_node->value, key);
	    database->dirty = true;
	  }
      }
  return success;
}

#if 0
/* key_possible_to_record -- Returns record with key
   str:  string that may be a key
   let:  if string starts with a letter, it must be this (eg, 'I' for indi)
   This returns NULL upon failure.  */

GNode *key_possible_to_record (String str, /* string that may be a key */
                    int let)    /* if string starts with letter it
                                   must be this */
{
	char kbuf[MAXGEDNAMELEN];
	int i = 0, c;

	if (!str || *str == 0) return NULL;
	c = *str++;
	if (c != let && chartype(c) != DIGIT) return NULL;
	kbuf[i++] = let;
	if (c != let) kbuf[i++] = c;
	while ((c = *str++) && chartype(c) == DIGIT)
		kbuf[i++] = c;
	if (c != 0) return NULL;
	kbuf[i] = 0;
	if (!isrecord(BTR, str2rkey(kbuf))) return NULL;
	switch (let) {
	case 'I': return qkey_to_irecord(kbuf);
	case 'F': return qkey_to_frecord(kbuf);
	case 'S': return qkey_to_srecord(kbuf);
	case 'E': return qkey_to_erecord(kbuf);
	case 'X': return qkey_to_orecord(kbuf);
	default:  FATAL();
	}
	FATAL();
	return NULL;
}
#endif

/* refn_to_record - Get record from user reference
   ukey: [IN]  refn key found
   letr: [IN]  possible type of record (0 if any)
   eg, refn_to_record("1850.Census", "S").  */

GNode *
refn_to_record (String ukey,    /* user refn key */
                int letr ATTRIBUTE_UNUSED,       /* type of record */
		Database *database)
{
  CString key = getRefn (ukey, database);
  if (! key)
    return NULL;

  return (getRecord (key, database->recordIndex));
}

/* getRefn -- searches the index for a mapping for refn, if found, returns it.
   If not found, null is returned.  */

CString
getRefn (CString refn, Database *database)
{
  return searchRefnIndex (database->refnIndex, refn);
}

/* traverseRefns -- traverses all refns in the index calling func on
   each with arguments (key, refn, param, database).  */

void
traverseRefns (TRAV_REFNS_FUNC func, void* param, Database *database)
{
  int bucket = 0;
  int element = 0;
  CString refn;
  CString key;
  RefnIndexEl *refn_elt;

  for (refn_elt = (RefnIndexEl *)firstInHashTable (database->refnIndex, &bucket, &element);
       refn_elt;
       refn_elt = nextInHashTable (database->refnIndex, &bucket, &element))
    {
      refn = refn_elt->refn;
      key = refn_elt->key;
      /* NOTE: all current callers always return true; but, the spec
	 says that they can return false to stop the iteration --
	 presumably if they 'found' what they were 'looking for'.  */
      if (! (*func)(key, refn, param, database))
	return;
    }
}

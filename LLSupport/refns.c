/* created by David Taylor */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "llnls.h"

#include "hashtable.h"
#include "refnindex.h"
#include "database.h"
#include "rfmt.h"

#include "refns.h"


struct tag_node_iter {
  GNode *start;
  GNode *next;
};

typedef struct tag_node_iter *NODE_ITER;
/* forward references */

static void begin_node_it (GNode *node, NODE_ITER nodeit);
static GNode *find_next (NODE_ITER nodeit);
static GNode *next_node_it_ptr (NODE_ITER nodeit);

/* resolveRefnLinks -- attempt to resolve the REFN links within node.
   When successful, replaces <something> with @something-else@.
   Returns the number of unresolved REFN links.  */

int
resolveRefnLinks (GNode *node, Database *database)
{
  int unresolved = 0;
  bool annotate_pointers = (getlloptint("AnnotatePointers", 0) > 0);
  GNode *child=0;
  struct tag_node_iter nodeit;

  if (!node) return 0;

  /* resolve all descendant nodes */
  begin_node_it(node, &nodeit);
  while ((child = next_node_it_ptr(&nodeit)) != NULL) {
    if (!resolve_node(child, annotate_pointers))
      ++unresolved;
  }
  return unresolved;
}


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
  insertInRefnIndex (database->refnIndex, refn, key);
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

  return true;
}

/* getRefn -- searches the index for a mapping for refn, if found, returns it.
   If not found, null is returned.  */

CString
getRefn (CString refn, Database *database)
{
  return searchRefnIndex (database->refnIndex, refn);
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
	  insertInRefnIndex (database->refnIndex, new_node->value, key);
      }
  return success;
}

/* annotate_with_supplemental -- Expand any references that have REFNs
   This converts, eg, "@S25@" to "<1850.Census>" Used for editing.  */

/* modifies tree in place -- perhaps it should copy the tree? */
void
annotateWithSupplemental (GNode *node, RFMT rfmt, Database *database)
{
  bool expand_refns = (getlloptint("ExpandRefnsDuringEdit", 0) > 0);
  bool annotate_pointers = (getlloptint("AnnotatePointers", 0) > 0);
  GNode *child=0;
  struct tag_node_iter nodeit;

  /* annotate all descendant nodes */
  begin_node_it(node, &nodeit);
  while ((child = next_node_it_ptr(&nodeit)) != NULL) {
    annotate_node(child, expand_refns, annotate_pointers, rfmt);
  }
}

/* traverseRefns -- traverses all refns in the index calling func on
   each with arguments (key, refn, param, database).  */

void
traverseRefns (TRAV_REFNS_FUNC func, Word param, Database *database)
{
}

/* begin_node_it -- Being a node iteration.  */

static void
begin_node_it (GNode *node, NODE_ITER nodeit)
{
	nodeit->start = node;
	/* first node to return is node */
	nodeit->next = node;
}

/* find_next -- Find next node in an ongoing iteration  */

static GNode *
find_next (NODE_ITER nodeit)
{
	GNode *curr = nodeit->next;
	/* goto child if there is one */
	GNode *next = nchild(curr);
	if (next)
		return next;
	/* otherwise try for sibling, going up to ancestors until
	we find one, or we hit the start & give up */
	while (1) {
		if (next == nodeit->start)
			return NULL;
		if (nsibling(curr))
			return nsibling(curr);
		curr = nparent(curr);
		if (!curr)
			return NULL;
	}

}

/* next_node_it_ptr -- Return next node in an ongoing iteration.  */

static GNode *
next_node_it_ptr (NODE_ITER nodeit)
{
	GNode *current = nodeit->next;
	if (current)
		nodeit->next = find_next(nodeit);
	return current;
}

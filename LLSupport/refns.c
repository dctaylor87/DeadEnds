/* created by David Taylor */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>

#include "porting.h"
#include "ll-porting.h"
#include "standard.h"
#include "llnls.h"

#include "hashtable.h"
#include "refnindex.h"
#include "gnode.h"
#include "database.h"
#include "zstr.h"
#include "rfmt.h"
#include "gstrings.h"
#include "gedcom.h"

#include "refns.h"
#include "llpy-externs.h"	/* XXX */

struct tag_node_iter {
  GNode *start;
  GNode *next;
};

typedef struct tag_node_iter *NODE_ITER;
/* forward references */

static void annotate_node (NODE node, BOOLEAN expand_refns,
			   BOOLEAN annotate_pointers, bool rfmt,
			   Database *database);
static BOOLEAN is_annotated_xref(CNSTRING val, INT * len);
static BOOLEAN resolve_node (NODE node, BOOLEAN annotate_pointers,
			     Database *database);
static STRING symbolic_link (CNSTRING val);

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
    if (!resolve_node(child, annotate_pointers, database))
      ++unresolved;
  }
  return unresolved;
}

/* resolve_node -- Traverse routine for resolve_refn_links (q.v.)
   node:    Current node in traversal
   returns FALSE if bad refn pointer.  */

static BOOLEAN
resolve_node (NODE node, BOOLEAN annotate_pointers, Database *database)
{
	STRING val = nval(node);
	STRING refn=0;

	if (!val) return TRUE;
	refn = symbolic_link(val);
	if (refn) {
		INT letr = record_letter(ntag(node));
		NODE refr = refn_to_record(refn, letr, database);
		if (refr) {
			stdfree(nval(node));
			nval(node) = strsave(nxref(refr));
		} else {
			return FALSE;
		}
	}
	if (annotate_pointers) {
		INT i=0,len=0;
		if (is_annotated_xref(nval(node), &len)) {
			char newval[20];
			ASSERT(len < (INT)sizeof(newval));
			for (i=0; i<len; ++i) {
				newval[i] = nval(node)[i];
			}
			newval[i] = 0;
			stdfree(nval(node));
			nval(node) = strsave(newval);
		}
	}

	return TRUE;
}
/* is_annotated_xref -- Return true if this is an annotated
   xref value (eg, "@I1@ {{ John/SMITH }}").  */

static BOOLEAN
is_annotated_xref (CNSTRING val, INT * len)
{
	CNSTRING ptr=val;
	INT end=0;
	if (!val) return FALSE;
	if (val[0] != '@') return FALSE;
	if (val[1] != 'I' && val[1] != 'F' && val[1] != 'S' 
		&& val[1] != 'E' && val[1] != 'X') return FALSE;
	if (!isdigit((uchar)val[2])) return FALSE;
	for (ptr = val + 3; isdigit((uchar)*ptr); ++ptr) {
	}
	if (ptr > val+9) return FALSE;
	if (*ptr++ != '@') return FALSE;
	if (ptr[0] != ' ') return FALSE;
	if (ptr[1] != '{') return FALSE;
	if (ptr[2] != '{') return FALSE;
	end = strlen(ptr);
	if (end < 3) return FALSE;
	if (ptr[end-1] != '}') return FALSE;
	if (ptr[end-2] != '}') return FALSE;
	*len = ptr-val;
	return TRUE;
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

/* annotateWithSupplemental -- Expand any references that have REFNs
   This converts, eg, "@S25@" to "<1850.Census>" Used for editing.  */

/* modifies tree in place -- perhaps it should copy the tree? */
void
annotateWithSupplemental (GNode *node, bool rfmt, Database *database)
{
  bool expand_refns = (getlloptint("ExpandRefnsDuringEdit", 0) > 0);
  bool annotate_pointers = (getlloptint("AnnotatePointers", 0) > 0);
  GNode *child=0;
  struct tag_node_iter nodeit;

  /* annotate all descendant nodes */
  begin_node_it(node, &nodeit);
  while ((child = next_node_it_ptr(&nodeit)) != NULL) {
    annotate_node(child, expand_refns, annotate_pointers, rfmt, database);
  }
}

/* annotate_node -- Alter a node by
   expanding refns (eg, "@S25@" to "<1850.Census>")
   annotating pointers (eg, "@I1@" to "@I1@ {{ John/SMITH }}")
   Used during editing.  */
static void
annotate_node (NODE node, BOOLEAN expand_refns,
	       BOOLEAN annotate_pointers, bool rfmt,
	       Database *database)
{
	STRING key=0;
	RECORD rec=0;

	key = value_to_xref(nval(node));
	if (!key) return;
	
	rec = key_possible_to_record(key, *key);
	if (!rec) return;
	
	if (expand_refns) {
		NODE refn = REFN(nztop(rec));
		char buffer[60];
		/* if there is a REFN, and it fits in our buffer,
		and it doesn't have any (confusing) > in it */
		if (refn && nval(refn) && !strchr(nval(refn), '>')
			&& strlen(nval(refn))<=sizeof(buffer)-3) {
			/* then replace, eg, @S25@, with, eg, <1850.Census> */
			buffer[0]=0;
			strcpy(buffer, "<");
			strcat(buffer, nval(refn));
			strcat(buffer, ">");
			stdfree(nval(node));
			nval(node) = strsave(buffer);
		}
	}

	if (annotate_pointers) {
		STRING str = generic_to_list_string(nztop(rec), key, 60, ", ", rfmt, FALSE, database);
		ZSTR zstr = zs_news(nval(node));
		zs_apps(zstr, " {{");
		zs_apps(zstr, str);
		zs_apps(zstr, " }}");
		stdfree(nval(node));
		nval(node) = strsave(zs_str(zstr));
		zs_free(&zstr);
	}

	/* release the (temporary) record created in key_possible_to_record() */
	release_record(rec);
}

/* symbolic_link -- See if value is symbolic link
   If so, returns heap-allocated copy of the reference
   (without surrounding angle brackets).  */
static STRING
symbolic_link (CNSTRING val)
{
	CNSTRING ptr=val;
	STRING link=0;
	INT len=0;
	if (!val || *val != '<') return NULL;
	len = strlen(val);
	if (len < 3) return FALSE;
	if (val[len-1] == '>') {
		/* entirely a symbolic link */
		link = strsave(val+1);
		link[len-2]=0;
		return link;
	}
	/* test for annotated symbolic link, that is, a line such as
	<a_ref_name> {{ James /SMITH/ }} */
	for (ptr=val+1; ptr[0]!='>'; ++ptr) {
		if (!ptr[0]) return NULL; /* no > at all */
	}
	if (ptr == val+1) return NULL; /* "<>" doesn't count */
	/* found end of symbolic link, see if annotation follows */
	if (ptr[1]!=' ' || ptr[2]!= '{' || ptr[3]!='{') return FALSE;
	if (val[len-2]!='}' || val[len-1]!='}') return FALSE;
	len = ptr-val;
	link = strsave(val+1);
	link[len-1]=0;
	return link;
}
/* record_letter -- Return letter for record type */

INT
record_letter (CNSTRING tag)
{
	if (eqstr("FATH", tag)) return 'I';
	if (eqstr("MOTH", tag)) return 'I';
	if (eqstr("HUSB", tag)) return 'I';
	if (eqstr("WIFE", tag)) return 'I';
	if (eqstr("INDI", tag)) return 'I';
	if (eqstr("CHIL", tag)) return 'I';
	if (eqstr("FAMC", tag)) return 'F';
	if (eqstr("FAMS", tag)) return 'F';
	if (eqstr("FAM",  tag)) return 'F';
	if (eqstr("SOUR", tag)) return 'S';
	if (eqstr("EVEN", tag)) return 'E';
	if (eqstr("EVID", tag)) return 'E';
	return 0;
}

#if 0
/* key_possible_to_record -- Returns record with key
   str:  string that may be a key
   let:  if string starts with a letter, it must be this (eg, 'I' for indi)
   This returns NULL upon failure.  */

RECORD key_possible_to_record (STRING str, /* string that may be a key */
                    INT let)    /* if string starts with letter it
                                   must be this */
{
	char kbuf[MAXGEDNAMELEN];
	INT i = 0, c;

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

NODE
refn_to_record (STRING ukey,    /* user refn key */
                INT letr,       /* type of record */
		Database *database)
{
	STRING *keys;
	INT num;

	if (!ukey || *ukey == 0) return NULL;
	get_refns(ukey, &num, &keys, letr);
	if (!num) return NULL;
	return nztop(key_possible_to_record(keys[0], *keys[0]));
}

/* traverseRefns -- traverses all refns in the index calling func on
   each with arguments (key, refn, param, database).  */

void
traverseRefns (TRAV_REFNS_FUNC func, Word param, Database *database)
{
  int bucket = 0;
  int element = 0;
  String refn;
  String key;
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

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdint.h>
#include <stdbool.h>
#include <locale.h>

#include "porting.h"
//#include "ll-porting.h"
#include "standard.h"

#include "sequence.h"
#include "gnode.h"
#include "zstr.h"
#include "de-strings.h"
#include "ll-sequence.h"

/* local function prototypes */

static void append_all_tags(Sequence *seq, GNode *node, CString tagname,
			    bool recurse, bool nonptrs);

/* append_all_tags -- append all tags of a specified tagname
   to Sequence (optionally recursive).  */

/* XXX this needs work -- not sure how val is intended to be used.  XXX */
static void
append_all_tags(Sequence *seq, GNode *node, CString tagname,
		bool recurse, bool nonptrs)
{
  if (!tagname || (ntag(node) && eqstr(ntag(node), tagname))) {
    CString key;
#if defined(DEADENDS)
    CString val = 0;
#else
    int val=0;
#endif

    key = nval(node);
    if (key && key[0]) {
      int keylen = strlen(key);
      String skey = 0;
      bool include=true;

      strupdate(&skey, rmvat(key));
      if (skey) {
#if defined(DEADENDS)
	val = skey;
#else
	val = atoi(skey+1);
#endif
      } else {
	if (nonptrs) {
	  ZSTR zstr = zs_newn(keylen+100);
	  GNode *chil;

#if defined(DEADENDS)
	  val = NULL;
#else
	  /* include non-pointers, but mark invalid with val==-1 */
	  val = -1;
#endif
	  zs_sets(zstr, key);
	  /* collect any CONC or CONT children */
	  for (chil = nchild(node); chil; chil=nsibling(chil)) {
	    String text = nval(chil) ? nval(chil) : "";
	    bool cr=false;

	    if (eqstr_ex(ntag(chil), "CONC")) {
	    } else if (eqstr_ex(ntag(chil), "CONT")) {
	      cr=true;
	    } else {
	      break;
	    }
	    if (cr)
	      zs_apps(zstr, "||");
	    zs_apps(zstr, text);
	  }
	  strupdate(&skey, zs_str(zstr));
	  zs_free(&zstr);
	} else {
	  include=false;
	}
      }
      if (include) {
#if defined(DEADENDS)
	appendToSequence (seq, skey, val);
#else
	append_indiseq_ival(seq, skey, NULL, val, false, true);
#endif
      }
    }
  }
  if (nchild(node) && recurse )
    append_all_tags(seq, nchild(node), tagname, recurse, nonptrs);
  if (nsibling(node))
    append_all_tags(seq, nsibling(node), tagname, recurse, nonptrs);
}

/* GNodeToSources -- Create a sequence of all sources
   mentioned inside a node tree (at any level).  */

Sequence *
GNodeToSources (GNode *node, Database *database)
{
  Sequence *seq;

  if (!node)
    return NULL;
  seq = createSequence(database);
  append_all_tags(seq, node, "SOUR", true, true);
  if (!lengthSequence(seq))
    {
      deleteSequence(seq);
      seq = NULL;
    }
  return seq;
}

/* GNodeToNotes -- Create a sequence of all notes
   inside a node tree (at any level).  */

Sequence *
GNodeToNotes (GNode *node, Database *database)
{
  Sequence *seq;

  if (!node)
    return NULL;

  seq = createSequence(database);
  append_all_tags(seq, node, "NOTE", true, true);
  if (! lengthSequence(seq))
    {
      deleteSequence(seq);
      seq = NULL;
    }
  return seq;
}

/* GNodeToPointers -- Create a sequence of all pointers
   inside a node tree (at any level).  */

Sequence *
GNodeToPointers (GNode *node, Database *database)
{
  Sequence *seq;

  if (!node)
    return NULL;

  seq = createSequence(database);
  append_all_tags(seq, node, NULL, true, false);
  if (! lengthSequence(seq))
    {
      deleteSequence(seq);
      seq = NULL;
    }
  return seq;
}

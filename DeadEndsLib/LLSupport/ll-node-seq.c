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

#include "database.h"
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
    CString val = 0;

    key = nval(node);
    if (key && key[0]) {
      int keylen = strlen(key);
      String skey = 0;
      bool include=true;

      strupdate(&skey, key);
      if (skey) {
	val = skey;
      } else {
	if (nonptrs) {
	  ZSTR zstr = zs_newn(keylen+100);
	  GNode *chil;

	  val = NULL;
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
	appendToSequence (seq, skey, val);
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
  seq = createSequence(database->recordIndex);
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

  seq = createSequence(database->recordIndex);
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

  seq = createSequence(database->recordIndex);
  append_all_tags(seq, node, NULL, true, false);
  if (! lengthSequence(seq))
    {
      deleteSequence(seq);
      seq = NULL;
    }
  return seq;
}

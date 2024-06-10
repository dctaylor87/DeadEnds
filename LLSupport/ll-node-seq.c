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

/* local function prototypes */

static void append_all_tags(Sequence *seq, GNode *node, CString tagname,
			    bool recurse, bool nonptrs);

/*=======================================================
 * append_all_tags -- append all tags of specified type
 *  to indiseq (optionally recursive
 * Created: 2000/11/29, Perry Rapp
 *=====================================================*/
static void
append_all_tags(Sequence *seq, GNode *node, CString tagname,
		bool recurse, bool nonptrs)
{
  if (!tagname || (ntag(node) && eqstr(ntag(node), tagname))) {
    String key;
    int val=0;
    key = nval(node);
    if (key && key[0]) {
      int keylen = strlen(key);
      String skey = 0;
      bool include=true;
      strupdate(&skey, rmvat(key));
      if (skey) {
	val = atoi(skey+1);
      } else {
	if (nonptrs) {
	  ZSTR zstr = zs_newn(keylen+100);
	  GNode *chil;
	  /* include non-pointers, but mark invalid with val==-1 */
	  val = -1;
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
	append_indiseq_ival(seq, skey, NULL, val, false, true);
      }
    }
  }
  if (nchild(node) && recurse )
    append_all_tags(seq, nchild(node), tagname, recurse, nonptrs);
  if (nsibling(node))
    append_all_tags(seq, nsibling(node), tagname, recurse, nonptrs);

}

/*=======================================================
 * node_to_sources -- Create sequence of all sources
 *  inside a node record (at any level)
 * Created: 2000/11/29, Perry Rapp
 *=====================================================*/
Sequence *
GNodeToSources (GNode *node, Database *database)
{
	Sequence *seq;
	if (!node) return NULL;
	seq = createSequence(database);
	append_all_tags(seq, node, "SOUR", true, true);
	if (!lengthSequence(seq))
	{
		deleteSequence(seq);
		seq = NULL;
	}
	return seq;
}

/*=======================================================
 * node_to_notes -- Create sequence of all notes
 *  inside a node record (at any level)
 * 2001/02/11, Perry Rapp
 *=====================================================*/
Sequence *
GNodeToNotes (GNode *node, Database *database)
{
	Sequence *seq;
	if (!node) return NULL;
	seq = createSequence(database);
	append_all_tags(seq, node, "NOTE", true, true);
	if (! lengthSequence(seq))
	{
		deleteSequence(seq);
		seq = NULL;
	}
	return seq;
}

/*=======================================================
 * node_to_pointers -- Create sequence of all pointers
 *  inside a node record (at any level)
 * 2001/02/24, Perry Rapp
 *=====================================================*/
Sequence *
GNodeToPointers (GNode *node, Database *database)
{
	Sequence *seq;
	if (!node) return NULL;
	seq = createSequence(database);
	append_all_tags(seq, node, NULL, true, false);
	if (! lengthSequence(seq))
	{
		deleteSequence(seq);
		seq = NULL;
	}
	return seq;
}

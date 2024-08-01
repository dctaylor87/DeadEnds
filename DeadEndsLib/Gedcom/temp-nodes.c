/* ll-temp-nodes.c -- these routines originated in LL, src/gedcom/nodes.c */

/* XXX currently this files lives in LLSupport.  But, it really should
   be in some other directory. XXX */

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdarg.h>
#include <stdint.h>

#include "porting.h"
#include "standard.h"
#include "llnls.h"

#include "zstr.h"
#include "list.h"
#include "refnindex.h"
#include "gnode.h"
#include "recordindex.h"
#include "hashtable.h"

/* createTempGNode -- Create GNode for temporary use
   (not to be connected to a record)
   [All arguments are duplicated, so caller doesn't have to]
   String xref  [in] xref
   String tag   [in] tag
   String val:  [in] value
   GNode *prnt: [in] parent
   Created: 2003-02-01 (Perry Rapp) */

GNode *
createTempGNode (String xref, String tag, String val, GNode *prnt)
{
	GNode *node = createGNode(xref, tag, val, prnt);
	nflag(node) = ND_TEMP;
	return node;
}

/* freeTempGNodeTree -- Free a node created by createTempGNode
   If the reference count is non-zero, we do not delete it nor its
   children -- siblings are still fair game, though.
   Created: 2003-02-01 (Perry Rapp).  Modified: David Taylor.  */

void
freeTempGNodeTree (GNode *node)
{
	GNode *n2;
	if (get_nrefcnt (node) == 0) {
		if ((n2 = nchild(node))) {
			freeTempGNodeTree(n2);
			nchild(node) = 0;
		}
	}
	if ((n2 = nsibling(node))) {
		freeTempGNodeTree(n2);
		nsibling(node) = 0;
	}
	if (get_nrefcnt (node) == 0) {
		freeGNode(node);
	}
}
/* isTempGNode -- Return whether node is a temp
   Created: 2003-02-04 (Perry Rapp) */

bool
isTempGNode (GNode *node)
{
	return !!(nflag(node) & ND_TEMP);
}

/* set_temp_node_helper -- {sets|clears} ND_TEMP in node, children,
   and siblings */

static void
set_temp_node_helper (GNode *node, bool temp)
{
  if (isTempGNode (node) ^ temp)
    nflag (node) ^= ND_TEMP;
  if (nchild (node))
    set_temp_node_helper (nchild (node), temp);
  if (nsibling (node))
    set_temp_node_helper (nsibling (node), temp);
}

/* setTempGNode -- make node temp (or not)
   Created: 2003-02-04 (Perry Rapp) */

void
setTempGNode (GNode *node, bool temp)
{
	if (isTempGNode(node) ^ temp)
		nflag(node) ^= ND_TEMP;
	if (nchild (node))
	  set_temp_node_helper (nchild (node), temp);
}

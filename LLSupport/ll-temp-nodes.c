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
#include "ll-porting.h"
#include "standard.h"
#include "llnls.h"

#include "zstr.h"
#include "list.h"
#include "refnindex.h"
#include "gnode.h"
#include "recordindex.h"
#include "hashtable.h"
#include "ll-node.h"

/* create_temp_node -- Create NODE for temporary use
   (not to be connected to a record)
   [All arguments are duplicated, so caller doesn't have to]
   STRING xref  [in] xref
   STRING tag   [in] tag
   STRING val:  [in] value
   NODE prnt:   [in] parent
   Created: 2003-02-01 (Perry Rapp) */

NODE
create_temp_node (STRING xref, STRING tag, STRING val, NODE prnt)
{
	NODE node = create_node(xref, tag, val, prnt);
	nflag(node) = ND_TEMP;
	return node;
}
/* free_temp_node_tree -- Free a node created by create_temp_node
   If the reference count is non-zero, we do not delete it nor its
   children -- siblings are still fair game, though.
   Created: 2003-02-01 (Perry Rapp).  Modified: David Taylor.  */

void
free_temp_node_tree (NODE node)
{
	NODE n2;
	if (get_nrefcnt (node) == 0) {
		if ((n2 = nchild(node))) {
			free_temp_node_tree(n2);
			nchild(node) = 0;
		}
	}
	if ((n2 = nsibling(node))) {
		free_temp_node_tree(n2);
		nsibling(node) = 0;
	}
	if (get_nrefcnt (node) == 0) {
		free_node(node,"free_temp_node_tree");
	}
}
/* is_temp_node -- Return whether node is a temp
   Created: 2003-02-04 (Perry Rapp) */

BOOLEAN
is_temp_node (NODE node)
{
	return !!(nflag(node) & ND_TEMP);
}

/* set_temp_node_helper -- {sets|clears} ND_TEMP in node, children,
   and siblings */

static void
set_temp_node_helper (NODE node, BOOLEAN temp)
{
  if (is_temp_node (node) ^ temp)
    nflag (node) ^= ND_TEMP;
  if (nchild (node))
    set_temp_node_helper (nchild (node), temp);
  if (nsibling (node))
    set_temp_node_helper (nsibling (node), temp);
}

/* set_temp_node -- make node temp (or not)
   Created: 2003-02-04 (Perry Rapp) */

void
set_temp_node (NODE node, BOOLEAN temp)
{
	if (is_temp_node(node) ^ temp)
		nflag(node) ^= ND_TEMP;
	if (nchild (node))
	  set_temp_node_helper (nchild (node), temp);
}

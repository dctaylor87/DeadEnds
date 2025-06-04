/* porting.c -- the purpose of this file iis to ease porting between
   the Python code in LifeLines and the Python code in DeadEnds.

   Whereas porting.h provides definitions, typedefs, and macros, this
   file provides small functions that don't exist in DeadEnds, but are
   simple enough that with a little glue they can call DeadEnds
   functions to do the bulk of the work.  */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "porting.h"

#include "hashtable.h"
#include "refnindex.h"
#include "database.h"
#include "recordindex.h"
#include "gnode.h"

#include "llpy-externs.h"

GNode *
_llpy_node_to_record (GNode *node, Database *database)
{
  GNode *top = node;

  while (top->parent)
    top = top->parent;

  if (! top->key)
    return NULL;		/* without the key we can't find the record! */

  GNode *record;

  record = ((GNode *)searchHashTable
	    ((HashTable *)database->recordIndex, top->key));
  return (record);
}

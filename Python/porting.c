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

#include "porting.h"

#include "hashtable.h"
#include "database.h"
#include "recordindex.h"

#include "llpy-externs.h"

RecordIndexEl *
_llpy_node_to_record (GNode *node)
{
  GNode *top = node;

  while (top->parent)
    top = top->parent;

  if (! top->key)
    return NULL;		/* without the key we can't find the record! */

#if 0
  RecordIndex *index;

  switch (top->key[0])
    {
    case 'I':
      index = theDatabase->personIndex;
      break;
    case 'F':
      index = theDatabase->familyIndex;
      break;
    case 'S':
      index = theDatabase->sourceIndex;
      break;
    case 'E':
      index = theDatabase->eventIndex;
      break;
    default:
      index = theDatabase->otherIndex;
      break;
    }
  return ((RecordIndexEl *)searchHashTable ((HashTable *)index, top->key));
#else
  /* we can no longer assume that the first character of the key tells
     us the record type, so we have to search ALL the key indexes */
  RecordIndexEl *record;

  record = ((RecordIndexEl *)searchHashTable
	    ((HashTable *)theDatabase->personIndex, top->key));
  if (! record)
    record = ((RecordIndexEl *)searchHashTable
	      ((HashTable *)theDatabase->familyIndex, top->key));
  if (! record)
    record = ((RecordIndexEl *)searchHashTable
	      ((HashTable *)theDatabase->sourceIndex, top->key));
  if (! record)
    record = ((RecordIndexEl *)searchHashTable
	      ((HashTable *)theDatabase->eventIndex, top->key));
  if (! record)
    record = ((RecordIndexEl *)searchHashTable
	      ((HashTable *)theDatabase->otherIndex, top->key));
  return (record);
#endif  
}

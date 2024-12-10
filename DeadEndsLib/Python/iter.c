/* iter.c -- iter types */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#include "porting.h"		/* LifeLines --> DeadEnds */
#include "standard.h"		/* String */
#include "denls.h"
#include "refnindex.h"
#include "gnode.h"		/* GNode */
#include "recordindex.h"	/* RecordIndexEl */

#include "python-to-c.h"
#include "types.h"

/* forward references */

/* these functions return iterators for all individuals, families, and
   sources, respectively, in the database.  */

static PyObject *llpy_events (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_individuals (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_families (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_sources (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_others (PyObject *self, PyObject *args, PyObject *kw);

static int nodeiter_next_child (LLINES_PY_NODEITER *iter);
static int nodeiter_next_traverse (LLINES_PY_NODEITER *iter);

static void llpy_record_iter_dealloc (PyObject *self);
static PyObject *llpy_record_iter (PyObject *self);
static PyObject *llpy_record_iternext (PyObject *self);

static void llpy_debug_print_node_iter (const char *prefix,
					LLINES_PY_NODEITER *self);
static void llpy_debug_print_node_iter_cur (const char *prefix,
					    LLINES_PY_NODEITER *iter);
/* start of code */

/* llpy_events (void) --> Returns an iterator for the set of
   events in the database.  */

static PyObject *llpy_events (PyObject *self,
			      PyObject *args, PyObject *kw)
{
  /* ALL non-db specific database iterators take an optional keyword
     argument: database.  If not None, then type needs to be
     llines_database_type, aka llines.Database. */

  static char *keywords[] = { "database", NULL };
  LLINES_PY_DATABASE *py_db = NULL;
  Database *database;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_events entry: self %p args %p\n",
	       (void *)self, (void *)args);
    }

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|O!", keywords,
				     &llines_database_type, &py_db))
    return NULL;

  /* database is either omitted, None, or valid */
  if (! py_db)
    database = currentDatabase;	/* need a better name for the default database */
  else
    database = py_db->lld_database;

  return _llpy_create_record_iterator (database, LLINES_TYPE_EVEN);
}

/* llpy_individuals (void) --> Returns an iterator for the set of
   individuals in the database.  */

static PyObject *llpy_individuals (PyObject *self,
				   PyObject *args, PyObject *kw)
{
  /* ALL non-db specific database iterators take an optional keyword
     argument: database.  If not None, then type needs to be
     llines_database_type, aka llines.Database. */

  static char *keywords[] = { "database", NULL };
  LLINES_PY_DATABASE *py_db = NULL;
  Database *database;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_individuals entry: self %p args %p\n",
	       (void *)self, (void *)args);
    }

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|O!", keywords,
				     &llines_database_type, &py_db))
    return NULL;

  /* database is either omitted, None, or valid */
  if (! py_db)
    database = currentDatabase;	/* need a better name for the default database */
  else
    database = py_db->lld_database;

  return _llpy_create_record_iterator (database, LLINES_TYPE_INDI);
}

/* llpy_families (void) --> Returns an iterator for the set of
   families in the database.  */

static PyObject *llpy_families (PyObject *self,
				PyObject *args, PyObject *kw)
{
  /* ALL non-db specific database iterators take an optional keyword
     argument: database.  If not None, then type needs to be
     llines_database_type, aka llines.Database. */

  static char *keywords[] = { "database", NULL };
  LLINES_PY_DATABASE *py_db = NULL;
  Database *database;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_families entry: self %p args %p\n",
	       (void *)self, (void *)args);
    }

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|O!", keywords,
				     &llines_database_type, &py_db))
    return NULL;

  /* database is either omitted, None, or valid */
  if (! py_db)
    database = currentDatabase;	/* need a better name for the default database */
  else
    database = py_db->lld_database;

  return _llpy_create_record_iterator (database, LLINES_TYPE_FAM);
}

/* llpy_sources (void) --> Returns an iterator for the set of sources
   in the database.  */

static PyObject *llpy_sources (PyObject *self,
			       PyObject *args, PyObject *kw)
{
  /* ALL non-db specific database iterators take an optional keyword
     argument: database.  If not None, then type needs to be
     llines_database_type, aka llines.Database. */

  static char *keywords[] = { "database", NULL };
  LLINES_PY_DATABASE *py_db = NULL;
  Database *database;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_sources entry: self %p args %p\n",
	       (void *)self, (void *)args);
    }

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|O!", keywords,
				     &llines_database_type, &py_db))
    return NULL;

  /* database is either omitted, None, or valid */
  if (! py_db)
    database = currentDatabase;	/* need a better name for the default database */
  else
    database = py_db->lld_database;

  return _llpy_create_record_iterator (database, LLINES_TYPE_SOUR);
}

/* llpy_others (void) --> Returns an iterator for the set of other records
   in the database.  */

static PyObject *llpy_others (PyObject *self,
			       PyObject *args, PyObject *kw)
{
  /* ALL non-db specific database iterators take an optional keyword
     argument: database.  If not None, then type needs to be
     llines_database_type, aka llines.Database. */

  static char *keywords[] = { "database", NULL };
  LLINES_PY_DATABASE *py_db = NULL;
  Database *database;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_others entry: self %p args %p\n",
	       (void *)self, (void *)args);
    }

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|O!", keywords,
				     &llines_database_type, &py_db))
    return NULL;

  /* database is either omitted, None, or valid */
  if (! py_db)
    database = currentDatabase;	/* need a better name for the default database */
  else
    database = py_db->lld_database;

  return _llpy_create_record_iterator (database, LLINES_TYPE_OTHR);
}

static void llpy_record_iter_dealloc (PyObject *self)
{
  if (llpy_debug)
    {
      fprintf (stderr, "llpy_record_iter_dealloc entry: self %p refcnt %ld\n",
	       (void *)self, Py_REFCNT (self));
    }
  PyObject_Del (self);
}

static PyObject *llpy_record_iter (PyObject *self)
{
  if (llpy_debug)
    {
      fprintf (stderr, "llpy_record_iter entry: self %p refcnt %ld, type %s\n",
	       (void *)self, Py_REFCNT (self), Py_TYPE(self)->tp_name);
    }
  Py_INCREF (self);
  return (self);
}

static PyObject *llpy_record_iternext (PyObject *self)
{
  LLINES_PY_RECORD_ITER *iter = (LLINES_PY_RECORD_ITER *)self;
  Database *database = iter->li_database;
  RecordIndexEl *record;
  RecordType DEtype;		/* GRPerson, GRFamily, GRSource, GREvent, or GROther */
  PyTypeObject *PYtype;		/* llines_{event,family,individual,other,source}_type */
  

  if (llpy_debug)
    {
      fprintf (stderr,
	       "llpy_record_iternext entry: type %c bucket %d element %d refcnt %ld type %s\n",
	       iter->li_type, iter->li_bucket_ndx, iter->li_element_ndx,
	       Py_REFCNT (self), Py_TYPE(self)->tp_name);
    }

  if (iter->li_bucket_ndx == -2)
    {
      /* trying to use an iter that was previously exhausted, raise an
	 exception */
      PyErr_SetObject (PyExc_StopIteration, Py_None);
      return NULL;
    }

  switch (iter->li_type)
    {
    case LLINES_TYPE_INDI:
      PYtype = &llines_individual_type;
      DEtype = GRPerson;
      break;
    case LLINES_TYPE_FAM:
      PYtype = &llines_family_type;
      DEtype = GRFamily;
      break;
    case LLINES_TYPE_SOUR:
      PYtype = &llines_source_type;
      DEtype = GRSource;
      break;
    case LLINES_TYPE_OTHR:
      PYtype = &llines_other_type;
      DEtype = GROther;
      break;
    case LLINES_TYPE_EVEN:
      PYtype = &llines_event_type;
      DEtype = GREvent;
      break;
    default:
      Py_UNREACHABLE ();	/* internal error */
    }

  if ((iter->li_bucket_ndx == -1) && (iter->li_element_ndx == -1))
    /* first time */
    record = (RecordIndexEl *) firstInHashTable (database->recordIndex,
						 &iter->li_bucket_ndx,
						 &iter->li_element_ndx);
  else
    /* not first time */
    record = (RecordIndexEl *) nextInHashTable (database->recordIndex,
						&iter->li_bucket_ndx,
						&iter->li_element_ndx);

  /* now, check the type, iterate until right type or exhausted */
  while (record && (recordType(record) != DEtype))
    {
      record = (RecordIndexEl *) nextInHashTable (database->recordIndex,
						  &iter->li_bucket_ndx,
						  &iter->li_element_ndx);
    }

  if (! record)
    {
      /* no record found of appropriate type -- exhausted */
      iter->li_bucket_ndx = -2;
      iter->li_element_ndx = -2;
      return NULL;
    }

  LLINES_PY_RECORD *obj = PyObject_New (LLINES_PY_RECORD, PYtype);
  if (! obj)
    {
      releaseRecord (record);
      return NULL;
    }

  obj->llr_database = database;
  obj->llr_record = record;
  obj->llr_type = iter->li_type;

  return (PyObject *)obj;
}

static void llpy_node_iter_dealloc (PyObject *self)
{
  LLINES_PY_NODEITER *nodeiter = (LLINES_PY_NODEITER *) self;

  if (llpy_debug)
    llpy_debug_print_node_iter ("llpy_node_iter_dealloc entry", nodeiter);

  if (nodeiter->ni_top_node) {
    nrefcnt_dec(nodeiter->ni_top_node);
    TRACK_NODE_REFCNT_DEC(nodeiter->ni_top_node);
  }
  nodeiter->ni_top_node = 0;

  if (nodeiter->ni_cur_node) {
    nrefcnt_dec(nodeiter->ni_cur_node);
    TRACK_NODE_REFCNT_DEC(nodeiter->ni_cur_node);
  }
  nodeiter->ni_cur_node = 0;

  if (nodeiter->ni_tag)
    free (nodeiter->ni_tag);
  nodeiter->ni_tag = 0;

  if (llpy_debug)
    llpy_debug_print_node_iter ("llpy_node_iter_dealloc exit", nodeiter);

  PyObject_Del (self);
}

static PyObject *llpy_node_iter (PyObject *self)
{
  if (llpy_debug)
    {
      fprintf (stderr, "llpy_node_iter entry: self %p refcnt %ld, type %s\n",
	       (void *)self, Py_REFCNT (self), Py_TYPE(self)->tp_name);
    }
  Py_INCREF (self);
  return (self);
}

static PyObject *llpy_node_iternext (PyObject *self)
{
  LLINES_PY_NODEITER *iter = (LLINES_PY_NODEITER *) self;
  LLINES_PY_NODE *py_node;
  int retval;

  /* these are only used if llpy_debug is true */

  /* these are for debugging ref counts which DeadEnds currently lacks */
  GNode *old_cur_node = iter->ni_cur_node;
  GNode *old_top_node = iter->ni_top_node;
#if 0
  /* if this is ever stored or returned, something is screwed up */
  int new_level = -2;
#endif

  if (llpy_debug)
    {
      fprintf (stderr,
	       "llpy_node_iternext entry: type %d current %p refcnt %ld, type %s\n",
	       iter->ni_type, (void *)iter->ni_cur_node, Py_REFCNT (self),
	       Py_TYPE(self)->tp_name);
      if (iter->ni_top_node)
	fprintf (stderr, "llpy_node_iternext: top %p, refcnt %d\n",
		 (void *)(iter->ni_top_node), get_nrefcnt(iter->ni_top_node));
      else
	fprintf (stderr, "llpy_node_iternext: top NULL\n");
      if (iter->ni_cur_node)
	fprintf (stderr, "llpy_node_iternext: cur %p, refcnt %d\n",
		 (void *)(iter->ni_cur_node), get_nrefcnt(iter->ni_cur_node));
      else
	fprintf (stderr, "llpy_node_iternext: cur NULL\n");
    }

  while (1)
    {
      if (iter->ni_level < 0)
	{
	  /* trying to use an iter that was previously exhausted, raise an
	     exception */
	  PyErr_SetObject (PyExc_StopIteration, Py_None);
	  return NULL;
	}

      if (iter->ni_type == ITER_CHILDREN)
	retval = nodeiter_next_child (iter);
      else if (iter->ni_type == ITER_TRAVERSE)
	retval = nodeiter_next_traverse (iter);
      else
	{
	  PyErr_SetString (PyExc_SystemError, "nodeiter_iternext: bad value for iter type");
	  return NULL;
	}
      if (retval < 0)
	{
	  /* previously exhausted */
	  PyErr_SetObject (PyExc_StopIteration, Py_None);
	  return NULL;
	}
      else if (retval == 0)
	return NULL;
      else
	{
	  /* we found a node, are we done? */
	  if ((iter->ni_tag == 0) || eqstr (ntag(iter->ni_cur_node), iter->ni_tag))
	    {
	      /* we are done */
	      py_node = PyObject_New (LLINES_PY_NODE, &llines_node_type);
	      if (! py_node)
		return NULL;
	      py_node->lnn_type = 0;
	      py_node->lnn_node = iter->ni_cur_node;
	      py_node->lnn_database = iter->ni_database;

	      nrefcnt_inc(py_node->lnn_node);
	      TRACK_NODE_REFCNT_INC(py_node->lnn_node);

	      if (llpy_debug)
		{
		  if (old_top_node)
		    fprintf (stderr, "llpy_node_iternext exit: old top refcnt %d\n",
			     get_nrefcnt(iter->ni_top_node));
		  if (old_cur_node)
		    fprintf (stderr, "llpy_node_iternext exit: old cur refcnt %d\n",
			     get_nrefcnt(iter->ni_cur_node));
		  llpy_debug_print_node_iter ("llpy_node_iternext exit", iter);
		}
	      return (PyObject *) py_node;
	    }
	}
    }
}

/* nodeiter_next_child -- helper function for llpy_node_iternext
   for the case of CHILD iteration, determine the next NODE to return.

   This function is ONLY concerned with returning the next node, not
   with the tag constraint, if any.

   return value: 1 -- normal, iter has NODE to return
                 0 -- iterator exhausted
		-1 -- iterator previously exhausted */

static int nodeiter_next_child (LLINES_PY_NODEITER *iter)
{
  if (iter->ni_level < 0)
    {
      PyErr_SetObject (PyExc_StopIteration, Py_None);
      return (-1);
    }

  if (! iter->ni_cur_node)
    {
      /* first iteration */
      iter->ni_cur_node = nchild (iter->ni_top_node);

      if (iter->ni_cur_node)
	{
	  nrefcnt_inc(iter->ni_cur_node);
	  TRACK_NODE_REFCNT_INC(iter->ni_cur_node);

	  if (llpy_debug)
	    llpy_debug_print_node_iter_cur ("nodeiter_next_child FIRST NEW cur", iter);

	  return (1);		/* we have a node */
	}
      else
	{
	  /* node we were asked to iterate over has no children! */
	  iter->ni_level = -1;	/* mark that it is exhausted */
	  return (0);		/* exhausted -- no children */
	}
    }

  /* not the first call */
  if (! iter->ni_cur_node)
    {
      /* paranoia -- should not happen */
      PyErr_SetString (PyExc_SystemError, "nodeiter_next_child: inconsistent iter");
      iter->ni_level = -2;
      return (-1);
    }
  nrefcnt_dec(iter->ni_cur_node);
  TRACK_NODE_REFCNT_DEC(iter->ni_cur_node);

  if (llpy_debug)
    llpy_debug_print_node_iter_cur ("nodeiter_next_child OLD cur", iter);

  /* check for siblings... */
  iter->ni_cur_node = nsibling (iter->ni_cur_node);

  if (iter->ni_cur_node)
    {
      nrefcnt_inc(iter->ni_cur_node);
      TRACK_NODE_REFCNT_INC(iter->ni_cur_node);

      if (llpy_debug)
	llpy_debug_print_node_iter_cur ("nodeiter_next_child exit NEW cur", iter);

      return (1);		/* we have a node */
    }

  iter->ni_level = -1;		/* mark that it is exhausted */

  if (llpy_debug)
    llpy_debug_print_node_iter_cur ("nodeiter_next_child exit NEW cur", iter);

  return (0);			/* exhausted -- no more children */
}

/* nodeiter_next_traverse -- helper function for llpy_node_iternext
   for the case of TRAVERSE iteration, determine the next NODE to
   return.

   This function is ONLY concerned with returning the next node,, not
   with the tag constraint, if any.

   return value: 1 -- normal, iter has NODE to return
                 0 -- iterator exhausted
		-1 -- iterator previously exhausted */

static int nodeiter_next_traverse (LLINES_PY_NODEITER *iter)
{
  GNode *new_node;
  int new_level;
  GNode *cur_node;

  if (iter->ni_level < 0)
    {
      PyErr_SetObject (PyExc_StopIteration, Py_None);
      return (-1);
    }

  if (! iter->ni_cur_node)
    {
      /* first iteration */
      iter->ni_cur_node = iter->ni_top_node;
      nrefcnt_inc(iter->ni_cur_node);
      TRACK_NODE_REFCNT_INC(iter->ni_cur_node);
      iter->ni_level = 0;
      return (1);		/* we have a node -- the top node */
    }

  /* not the first call */
  new_node = nchild (iter->ni_cur_node);
  new_level = iter->ni_level + 1;

  if (new_node)
    {
      nrefcnt_dec(iter->ni_cur_node);
      TRACK_NODE_REFCNT_DEC(iter->ni_cur_node);
      iter->ni_cur_node = new_node;
      nrefcnt_inc(iter->ni_cur_node);
      TRACK_NODE_REFCNT_INC(iter->ni_cur_node);
      iter->ni_level = new_level;
      return (1);
    }

  /* no more desendants, try a sibling */
  new_node = nsibling (iter->ni_cur_node);
  new_level--;

  if (new_node)
    {
      nrefcnt_dec(iter->ni_cur_node);
      TRACK_NODE_REFCNT_DEC(iter->ni_cur_node);
      iter->ni_cur_node = new_node;
      nrefcnt_inc(iter->ni_cur_node);
      TRACK_NODE_REFCNT_INC(iter->ni_cur_node);
      iter->ni_level = new_level;
      return (1);
    }
  /* nothing below, nothing to the side, so go up */

  /* start of loop */

  cur_node = iter->ni_cur_node;

  while (1)
    {
      cur_node = nparent (cur_node);
      new_level--;

      /* pararnoia -- these conditions should either both be true or
	 both be false */
      if ((cur_node == iter->ni_top_node) ||
	  (new_level <= 0))
	{
	  /* got back to the top so we are exhausted */
	  iter->ni_level = -1;
	  return (0);		/* newly exhausted */
	}
      new_node = nsibling (cur_node);
      if (new_node)
	{
	  /* found one, return it */
	  nrefcnt_dec(iter->ni_cur_node);
	  TRACK_NODE_REFCNT_DEC(iter->ni_cur_node);
	  iter->ni_cur_node = new_node;
	  nrefcnt_inc(iter->ni_cur_node);
	  TRACK_NODE_REFCNT_INC(iter->ni_cur_node);
	  iter->ni_level = new_level;
	  return (1);
	}
    }
}

static void llpy_debug_print_node_iter (const char *prefix,
					LLINES_PY_NODEITER *iter)
{
  fprintf (stderr, "%s: self %p\n", prefix, (void *) iter);
  fprintf (stderr,
	   "%s: search type %d top %p cur %p refcnt %ld, type name %s\n",
	   prefix, iter->ni_type, (void *)iter->ni_top_node,
	   (void *)iter->ni_cur_node, Py_REFCNT (iter),
	   Py_TYPE(iter)->tp_name);

  if (iter->ni_top_node)
    fprintf (stderr, "%s: top refcnt %d xref %s tag %s val %s\n",
	     prefix, get_nrefcnt(iter->ni_top_node),
	     nxref(iter->ni_top_node) ? nxref(iter->ni_top_node) : "NULL",
	     ntag(iter->ni_top_node),
	     nval(iter->ni_top_node) ? nval(iter->ni_top_node) : "NULL");
  if (iter->ni_cur_node)
    fprintf (stderr, "%s: cur refcnt %d xref %s tag %s val %s\n",
	     prefix, get_nrefcnt(iter->ni_cur_node),
	     nxref(iter->ni_cur_node) ? nxref(iter->ni_cur_node) : "NULL",
	     ntag(iter->ni_cur_node),
	     nval(iter->ni_cur_node) ? nval(iter->ni_cur_node) : "NULL");
  else
    fprintf (stderr, "%s: cur NULL\n", prefix);
}

/* llpy_debug_print_node_iter_cur -- just print ni_cur_node, after
   delta before changing */
static void llpy_debug_print_node_iter_cur (const char *prefix,
					    LLINES_PY_NODEITER *iter)
{
  if (iter->ni_cur_node)
    fprintf (stderr, "%s: cur refcnt %d xref %s tag %s val %s\n",
	     prefix, get_nrefcnt(iter->ni_cur_node),
	     nxref(iter->ni_cur_node) ? nxref(iter->ni_cur_node) : "NULL",
	     ntag(iter->ni_cur_node),
	     nval(iter->ni_cur_node) ? nval(iter->ni_cur_node) : "NULL");
  else
    fprintf (stderr, "%s: cur NULL\n", prefix);
}

static struct PyMethodDef Lifelines_Iter_Functions[] =
  {
   /* RECORD functions */

   { "individuals",	(PyCFunction)llpy_individuals, METH_VARARGS | METH_KEYWORDS,
     "individuals([database]) -> iterator for the set of all INDI in the database" },
   { "families",	(PyCFunction)llpy_families, METH_VARARGS | METH_KEYWORDS,
     "families([database]) -> iterator for the set of all FAM in the database" },
   { "sources",		(PyCFunction)llpy_sources, METH_VARARGS | METH_KEYWORDS,
     "sources([database]) -> iterator for the set of all SOUR in the database" },
   { "events",		(PyCFunction)llpy_events, METH_VARARGS | METH_KEYWORDS,
     "events([database]) --> iterator for the set of all EVEN in the database" },
   { "others",		(PyCFunction)llpy_others, METH_VARARGS | METH_KEYWORDS,
     "others([database]) --> iterator for the set of all OTHR records in the database" },

   /* NODE functions */

   { NULL, 0, 0, NULL }		/* sentinel */
  };

PyTypeObject llines_record_iter_type =
  {
   PyVarObject_HEAD_INIT (NULL, 0)
   .tp_name = "llines.Iter",
   .tp_doc = "Lifelines GEDCOM Iteration Type",
   .tp_basicsize = sizeof (LLINES_PY_RECORD_ITER),
   .tp_itemsize = 0,
   .tp_flags = Py_TPFLAGS_DEFAULT,
   .tp_new = PyType_GenericNew,
   .tp_dealloc = llpy_record_iter_dealloc,
   .tp_methods = NULL,
   .tp_iter = llpy_record_iter,
   .tp_iternext = llpy_record_iternext,
  };

PyTypeObject llines_node_iter_type =
  {
   PyVarObject_HEAD_INIT(NULL, 0)
   .tp_name = "llines.NodeIter",
   .tp_doc = "Lifelines GEDCOM NODE Iteration Type",
   .tp_basicsize = sizeof (LLINES_PY_NODEITER),
   .tp_itemsize = 0,
   .tp_flags = Py_TPFLAGS_DEFAULT,
   .tp_new = PyType_GenericNew,
   .tp_dealloc = llpy_node_iter_dealloc,
   .tp_methods = NULL,
   .tp_iter = llpy_node_iter,
   .tp_iternext = llpy_node_iternext,
  };

void llpy_iter_init (void)
{
  int status;

  status = PyModule_AddFunctions (Lifelines_Module, Lifelines_Iter_Functions);

  if (status != 0)
    fprintf (stderr, "llpy_iter_init: attempt to add functions returned %d\n", status);

  status = PyModule_AddIntMacro (Lifelines_Module, ITER_CHILDREN);

  if (status != 0)
    fprintf (stderr, "llpy_iter_init: attempt to add macro ITER_CHILDREN returned %d\n", status);

  status = PyModule_AddIntMacro (Lifelines_Module, ITER_TRAVERSE);

  if (status != 0)
    fprintf (stderr, "llpy_iter_init: attempt to add macro ITER_TRAVERSE returned %d\n", status);
}

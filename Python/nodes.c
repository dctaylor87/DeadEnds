#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>

#if defined(DEADENDS)
#include "porting.h"		/* LifeLines --> DeadEnds */
#include "standard.h"		/* String */
#include "llnls.h"
#include "refnindex.h"
#include "gnode.h"		/* GNode */
#include "recordindex.h"	/* RecordIndexEl */
#else
#include "standard.h"		/* STRING */
#include "llstdlib.h"
#include "gedcom.h"
#include "../interp/interpi.h"	/* XXX */
#include "../gedlib/leaksi.h"
#endif

#include "types.h"
#include "python-to-c.h"

#include "event.h"

/* forward references */

static PyObject *llpy_xref (PyObject *self, PyObject *args);
static PyObject *llpy_tag (PyObject *self, PyObject *args);
static PyObject *llpy_value (PyObject *self, PyObject *args);
static PyObject *llpy_parent_node (PyObject *self, PyObject *args);
static PyObject *llpy_child_node (PyObject *self, PyObject *args);
static PyObject *llpy_sibling_node (PyObject *self, PyObject *args);
static PyObject *llpy_copy_node_tree (PyObject *self, PyObject *args);
static PyObject *llpy_level (PyObject *self, PyObject *args);
static PyObject *llpy_nodeiter (PyObject *self, PyObject *args, PyObject *kw);
static void llpy_debug_print_node (const char *prefix, PyObject *self);

/* start of code */

static PyObject *llpy_xref (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *node = (LLINES_PY_NODE *) self;

  return Py_BuildValue ("s", nxref (node->lnn_node));
}

static PyObject *llpy_tag (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *node = (LLINES_PY_NODE *) self;

  return Py_BuildValue ("s", ntag (node->lnn_node));
}

static PyObject *llpy_value (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{

  LLINES_PY_NODE *node = (LLINES_PY_NODE *) self;
  String str = nval (node->lnn_node);

  if (! str)
    Py_RETURN_NONE;		/* node has no value -- probably has subnodes */

  return Py_BuildValue ("s", str);
}

static PyObject *llpy_parent_node (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *node = (LLINES_PY_NODE *) self;
  LLINES_PY_NODE *parent;
  NODE pnode = nparent (node->lnn_node);

  if (! pnode)
    Py_RETURN_NONE;		/* No parent -- already top of the tree */

  parent = PyObject_New (LLINES_PY_NODE, &llines_node_type);
  if (! parent)
    return NULL;		/* PyObject_New failed and set an exception */

  nrefcnt_inc(pnode);
  TRACK_NODE_REFCNT_INC(pnode);
  parent->lnn_type = node->lnn_type; /* 'inherit' type from previous node */
  parent->lnn_database = node->lnn_database;
  parent->lnn_node = pnode;

  return (PyObject *)parent;
}

static PyObject *llpy_child_node (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *py_node = (LLINES_PY_NODE *) self;
  LLINES_PY_NODE *new_py_node;
  NODE node = py_node->lnn_node;

  node = nchild (node);
  if (! node)
    Py_RETURN_NONE;		/* node has no children */

  new_py_node = PyObject_New (LLINES_PY_NODE, &llines_node_type);
  if (! new_py_node)
    return NULL;

  nrefcnt_inc(node);
  TRACK_NODE_REFCNT_INC(node);

  new_py_node->lnn_type = py_node->lnn_type;
  new_py_node->lnn_database = py_node->lnn_database;
  new_py_node->lnn_node = node;

  return (PyObject *)new_py_node;
}

static PyObject *llpy_sibling_node (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *py_node = (LLINES_PY_NODE *) self;
  LLINES_PY_NODE *new_py_node;
  NODE node = py_node->lnn_node;

  node = nsibling (node);
  if (! node)
    Py_RETURN_NONE;		/* node has no siblings */

  new_py_node = PyObject_New (LLINES_PY_NODE, &llines_node_type);
  if (! py_node)
    return NULL;

  nrefcnt_inc(node);
  TRACK_NODE_REFCNT_INC(node);
  new_py_node->lnn_type = py_node->lnn_type;
  new_py_node->lnn_database = py_node->lnn_database;
  new_py_node->lnn_node = node;

  return (PyObject *)new_py_node;
}

static PyObject *llpy_copy_node_tree (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *node = (LLINES_PY_NODE *) self;
  LLINES_PY_NODE *copy = PyObject_New (LLINES_PY_NODE, &llines_node_type);

  if (! copy)
    return NULL;

  copy->lnn_type = node->lnn_type;
  copy->lnn_database = node->lnn_database;
  copy->lnn_node = copy_nodes (node->lnn_node, TRUE, TRUE);
  nrefcnt_inc(copy->lnn_node);
  TRACK_NODE_REFCNT_INC(copy->lnn_node);

  return (PyObject *)copy;
}

static PyObject *llpy_level (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *node = (LLINES_PY_NODE *) self;
  INT level = -1;

  for (NODE llnode = node->lnn_node; llnode; llnode = nparent (llnode))
    level++;

  return Py_BuildValue ("i", level);
}

/* llpy_add_node([parent], [prev] -- insert current node into GEDCOM
   tree with specified parent and previous sibling. */

static PyObject *llpy_add_node (PyObject *self, PyObject *args, PyObject *kw)
{
  LLINES_PY_NODE *orig = (LLINES_PY_NODE *) self;
  NODE orig_node = orig->lnn_node;
  static char *keywords[] = { "parent", "prev", NULL };
  PyObject *parent= Py_None;
  PyObject *prev = Py_None;
  NODE parent_node = NULL;
  NODE prev_node = NULL;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|OO", keywords, &parent, &prev))
    return NULL;

  if (parent == Py_None)
    /* either we will become the top-node or we'll get our parent later... */
    parent_node = NULL;
  else if (Py_TYPE(parent) == &llines_node_type)
    parent_node = ((LLINES_PY_NODE *) parent)->lnn_node;
  else
    {
      /* error -- wrong type -- set exception and return NULL */
      PyErr_SetString (PyExc_TypeError, "add_node: parent must be a NODE or None");
      return NULL;
    }
  if (prev == Py_None)
    /* either we are first in line or our preceding sibling will be added later... */
    prev_node = NULL;
  else if (Py_TYPE(prev) == &llines_node_type)
    prev_node = ((LLINES_PY_NODE *) prev)->lnn_node;
  else
    {
      /* error -- wrong type -- set exception and return NULL */
      PyErr_SetString (PyExc_TypeError, "add_node: prev must be a NODE or None");
      return NULL;
    }

  if (prev_node && (nparent(prev_node) != parent_node))
    {
      /* prev_node has a different parent! -- set exception and return
	 NULL */
      PyErr_SetString (PyExc_ValueError, "add_node: prev is not a child of parent");
      return NULL;
    }

#if defined(DEADENDS)
  nparent (orig_node) = parent_node;
  set_temp_node (orig_node, is_temp_node (parent_node));
#else
  /* XXX the following lines bracketed by dolock_node_in_cache need to
     be double and triple checked XXX */

  dolock_node_in_cache (orig_node, FALSE);
  nparent (orig_node) = parent_node;
  if (parent_node)
    orig_node->n_cel = parent_node->n_cel;
  else
    orig_node->n_cel = NULL;	/* XXX double check this! XXX */

  set_temp_node (orig_node, is_temp_node (parent_node));
  dolock_node_in_cache (orig_node, TRUE);
#endif

  /* if both prev_node and parent_node are NULL, then there is nothing to do */
  if (! prev_node)
    {
      if (! parent_node)
	;			/* neither a parent nor a sibling -- nothing to do */
      else
	{
	  /* parent, but no previous sibling -- we are first in line */
	  nsibling(orig_node) = nchild (parent_node);
	  nchild (parent_node) = orig_node;
	}
    }
  else
    {
      if (parent_node)
	{
	  /* have sibling and parent */
	  nparent (orig_node) = parent_node;
	  nsibling (orig_node) = nsibling (prev_node);
	  nsibling (prev_node) = orig_node;
	}
      else
	{
	  /* have sibling, but no parent -- extend the line */
	  nsibling (orig_node) = nsibling(prev_node);
	  nsibling(prev_node) = orig_node;
	}
    }
  /* parent_node might be a permanent node or a temporary node -- set
     ND_TEMP flag of orig_node (and children) to match that of
     parent_node */
  set_temp_node (orig_node, is_temp_node (parent_node));

  return (self);
}

/* llpy_detach_node (void) --> NODE

   Removes NODE from its GEDCOM tree.  Afterwards the node is marked
   as a temporary node.  Currently fails (and raises an exception) if
   NODE is the top node */
static PyObject *llpy_detach_node (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  NODE node = ((LLINES_PY_NODE *) self)->lnn_node;
  NODE parent = nparent(node);

  if (! parent)
    {
      /* not currently supported -- possibly in the future? */
      PyErr_SetString (PyExc_ValueError, "detach_node: detaching top-node of a record is not (currently?) supported");
      return NULL;
    }
  NODE previous = NULL;
  NODE current = nchild (parent);

  /* walk list of immediate children of parent looking for node
     being detached */
  while (current && current != node)
    {
      previous = current;
      current = nsibling (current);
    }
  if (! current)
    {
      /* tree is broken -- node is not a child of its parent! --
	 internal error */
      PyErr_SetString (PyExc_SystemError, "llpy_detach_node: node tree is inconsistent");
      return NULL;
    }
  NODE next = nsibling (node);
  if (previous == NULL)
    nchild (parent) = next;	/* our node is the first child */
  else
    nsibling (previous) = next; /* not the first child */

#if defined(DEADENDS)
  /* DeadEnds has no cache, so no need to lock it in the cache */
  nparent (node) = NULL;
#else
  /* unparent node, but ensure its locking is only releative to new parent */
  dolock_node_in_cache (node, FALSE);
  nparent (node) = NULL;
  dolock_node_in_cache (node, TRUE);
#endif

  nsibling (node) = NULL;

  /* whether it was permanent before or not, it is now temporary */
  set_temp_node (node, TRUE);

  return (self);
}


/* llpy_create_node (tag,[value]) --> NODE Returns the newly created
   node having specified TAG.  If VALUE is omitted, None, or empty,
   then the value is empty.*/
static PyObject *llpy_create_node (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "tag", "value", NULL };
  String xref = 0;
  String tag = 0;
  String value = 0;
  NODE parent = 0;
  NODE node;
  LLINES_PY_NODE *py_node;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "s|z", keywords, &tag, &value))
    return NULL;

  node = create_temp_node (xref, tag, value, parent);
  if (! (py_node = PyObject_New(LLINES_PY_NODE, &llines_node_type)))
    return NULL;

  nrefcnt_inc(node);
  TRACK_NODE_REFCNT_INC(node);
  py_node->lnn_node = node;
  py_node->lnn_database = 0;	/* not currently part of any database! */
  py_node->lnn_type = 0;	/* unknown */

  return ((PyObject *)py_node);
}

/* llpy_nodeiter -- returns an iterator for the input node tree.

   The 'type' of iterator depends on the input arguments.

   If TYPE is CHILD, we iterate over the immediate children of the
   input node.

   If TYPE is TRAVERSE, we iterate over all the nodes, depth first,
   parent before child.

   TAG is either None or a string.  If it is None, then we behave as
   above.  If it is a string, then we behave as above *BUT* before
   returning a node, we compare its tag against the string.  If they
   match we return it, if they do not match, we continue the iteration
   until we find a match or we exhaust the iteration.  */

static PyObject *llpy_nodeiter (PyObject *self, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "type", "tag", NULL };
  int type = 0;
  char *tag = 0;
  LLINES_PY_NODEITER *iter;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "i|z", keywords, &type, &tag))
    return NULL;

  if ((type != ITER_CHILDREN) && (type != ITER_TRAVERSE))
    {
      PyErr_SetString (PyExc_ValueError, "nodeiter: type must be either ITER_CHILDREN or ITER_TRAVERSE");
      return NULL;
    }
  iter = PyObject_New (LLINES_PY_NODEITER, &llines_node_iter_type);
  if (! iter)
    return NULL;

  /*Py_INCREF (self);*/
  iter->ni_top_node = ((LLINES_PY_NODE *)self)->lnn_node;
  nrefcnt_inc(iter->ni_top_node);
  TRACK_NODE_REFCNT_INC(iter->ni_top_node);
  iter->ni_cur_node = NULL;
  iter->ni_type = type;
  iter->ni_level = 0;
  iter->ni_database = ((LLINES_PY_NODE *)self)->lnn_database;
  if ((tag == 0) || (*tag == '\0'))
    iter->ni_tag = 0;
  else
    iter->ni_tag = strdup (tag);

  return (PyObject *) iter;
}

static void llpy_node_dealloc (PyObject *self)
{
  LLINES_PY_NODE *node = (LLINES_PY_NODE *) self;

  if (llpy_debug)
    llpy_debug_print_node ("llpy_node_dealloc entry", self);

  nrefcnt_dec(node->lnn_node);
  TRACK_NODE_REFCNT_DEC(node->lnn_node);
  node->lnn_node = 0;
  node->lnn_type = 0;

  if (llpy_debug)
    llpy_debug_print_node ("llpy_node_dealloc exit", self);

  Py_TYPE(self)->tp_free (self);
}

static void llpy_debug_print_node (const char *prefix, PyObject *self)
{
  NODE node;

  fprintf (stderr, "%s: self %p refcnt %ld type %s\n",
	   prefix, (void *)self, Py_REFCNT (self), Py_TYPE (self)->tp_name);

  node = ((LLINES_PY_NODE *) self)->lnn_node;

  if (! node)
    fprintf (stderr, "%s: lnn_node: node NULL\n", prefix);
  else
#if defined(DEADENDS)
    fprintf (stderr, "%s: lnn node: node %p tag %s\n",
	     prefix, (void *)node, ntag(node));
#else
    fprintf (stderr, "%s: lnn node: node %p tag %s refcnt %d\n",
	     prefix, (void *)node, ntag(node), nrefcnt(node));
#endif
}

static struct PyMethodDef Lifelines_Node_Methods[] =
  {
   /* NODE Functions */

   { "xref",		(PyCFunction)llpy_xref, METH_NOARGS,
     "(NODE).xref(void) --> STRING; returns cross reference index of NODE" },
   { "tag",		(PyCFunction)llpy_tag, METH_NOARGS,
     "(NODE).tag(void) --> STRING; returns tag of NODE" },
   { "value",		(PyCFunction)llpy_value, METH_NOARGS,
     "(NODE).value(void) --> STRING; returns value of NODE" },
   { "parent_node",	(PyCFunction)llpy_parent_node, METH_NOARGS,
     "(NODE).parent_node(void) --> NODE; returns parent node of NODE" },
   { "child_node",	(PyCFunction)llpy_child_node, METH_NOARGS,
     "(NODE).child_node(void) --> NODE; returns first child of NODE" },
   { "sibling_node",	(PyCFunction)llpy_sibling_node, METH_NOARGS,
     "(NODE).sibling_node(void) --> NODE; returns next sibling of NODE" },
   { "copy_node_tree",	(PyCFunction)llpy_copy_node_tree, METH_NOARGS,
     "(NODE).copy_node_tree(void) --> NODE; returns a copy of the node structure" },
   { "level",		(PyCFunction)llpy_level, METH_NOARGS,
     "(NODE).level(void) --> INT; returns the level of NODE" },

   { "add_node",	(PyCFunction)llpy_add_node, METH_VARARGS | METH_KEYWORDS,
     "(NODE).add_node([parent],[prev]) --> NODE.  Modifies node to have\n\
parent PARENT and previous sibling PREV.  Returns modified NODE" },
   { "detach_node",	(PyCFunction)llpy_detach_node, METH_NOARGS,
     "(NODE).detach_node(void) --> NODE.  Detach NODE from its tree.\n\n\
It is (currently) an error for it to be the top-node of the tree." },
   { "nodeiter",	(PyCFunction)llpy_nodeiter, METH_VARARGS | METH_KEYWORDS,
     "nodeiter(type, [tag]) --> Iterator over node tree.\n\n\
TYPE is an int -- one of ITER_CHILDREN or ITER_TRAVERSE.\n\
TAG, if specified, restricts the iterator to just those nodes having that tag." },

   { "date",		llpy_date, METH_NOARGS,
     "(NODE).date(void) -> STRING: value of first DATE line for NODE." },
   { "place",		llpy_place, METH_NOARGS,
     "(NODE).place(void) -> STRING: value of first PLAC line for NODE." },
   { "year",		llpy_year, METH_NOARGS,
     "(NODE).year(void) --> STRING: year or first string of 3-4 digits in DATE line of NODE." },
   { "long",		llpy_long, METH_NOARGS,
     "(NODE).long(void) --> STRING: values of first DATE and PLAC lines of NODE" },
   { "short",		llpy_short, METH_NOARGS,
     "(NODE).short(void) --> STRING: abbreviated values of DATE and PLAC lines of NODE." },
   { "stddate",		llpy_stddate_node, METH_NOARGS,
     "(NODE).stddate(void) --> STRING: formatted date string." },
   { "complexdate",	llpy_complexdate_node, METH_NOARGS,
     "(NODE).complexdate(void) --> STRING; Formats and returns NODEs date\n\
using complex date formats previously specified." },

   { NULL, 0, 0, NULL }		/* sentinel */
  };

static struct PyMethodDef Lifelines_Node_Functions[] =
  {
   { "create_node",	(PyCFunction)llpy_create_node, METH_VARARGS | METH_KEYWORDS,
     "create_node(tag,[value]) --> NODE.  Creates node having tag and value." },

   { NULL, 0, 0, NULL }		/* sentinel */
  };

PyTypeObject llines_node_type =
  {
   PyVarObject_HEAD_INIT(NULL, 0)
   .tp_name = "llines.Node",
   .tp_doc = "Lifelines GEDCOM generic Node",
   .tp_basicsize = sizeof (LLINES_PY_NODE),
   .tp_itemsize = 0,
   .tp_flags = Py_TPFLAGS_DEFAULT,
   .tp_new = PyType_GenericNew,
   .tp_dealloc = llpy_node_dealloc,
   .tp_methods = Lifelines_Node_Methods,
  };

void llpy_nodes_init (void)
{
  int status;

  status = PyModule_AddFunctions (Lifelines_Module, Lifelines_Node_Functions);
  if (status != 0)
    fprintf (stderr, "llpy_nodes_init: attempt to add functions returned %d\n", status);
}



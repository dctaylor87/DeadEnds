#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */

#if defined(DEADENDS)
#include "porting.h"		/* LifeLines --> DeadEnds */
#include "standard.h"		/* String */
#include "llnls.h"
#include "refnindex.h"
#include "gnode.h"		/* GNode */
#include "recordindex.h"	/* RecordIndexEl */
#else
#include "llstdlib.h"
#include "gedcom.h"
#include "pvalue.h"
#endif

#include "types.h"

#include "llpy-externs.h"
#include "python-to-c.h"

/* forward references */

static void llpy_record_dealloc (PyObject *self);

/* llines_record_richcompare -- Sorting rules:

     RECORDs < NODEs < everything else

     For comparisions between two RECORDs or two NODEs:
     major sort on type, minor sort on keynum.  */

PyObject *
llines_record_richcompare (PyObject *self, PyObject *other, int op)
{
  LLINES_PY_RECORD *obj1;
  LLINES_PY_RECORD *obj2;

  /* determine if 'other' is a RECORD, a NODE, or something else.
     While *we* are only going to create sets where everything is a
     RECORD or everything is a NODE, the user can create any sort of
     set he/she wishes to create, so we need to handle that.  */
  if ((other->ob_type == &llines_family_type) ||
      (other->ob_type == &llines_individual_type) ||
      (other->ob_type == &llines_source_type))
    {

      obj1 = (LLINES_PY_RECORD *) self;
      obj2 = (LLINES_PY_RECORD *) other;

      if (nztype (obj1->llr_record) == nztype (obj2->llr_record))
#if defined(DEADENDS)
	{
	  int keynum1 = atoi (&(obj1->llr_record->root->key[1]));
	  int keynum2 = atoi (&(obj2->llr_record->root->key[1]));

	  Py_RETURN_RICHCOMPARE (keynum1, keynum2, op);
	}
#else
	Py_RETURN_RICHCOMPARE (nzkeynum (obj1->llr_record), nzkeynum (obj2->llr_record), op);
#endif
      else
	Py_RETURN_RICHCOMPARE (nztype (obj1->llr_record), nztype (obj2->llr_record), op);
    }
  else
    {
      switch (op)
	{
	case Py_LE:
	case Py_LT:
	case Py_NE:
	  Py_RETURN_TRUE;
	case Py_GE:
	case Py_GT:
	case Py_EQ:
	  Py_RETURN_FALSE;
	default:
	  Py_UNREACHABLE();
	}
    }
}

PyObject *
llines_node_richcompare (PyObject *self, PyObject *other, int op)
{
  RECORD obj1;
  RECORD obj2;

  /* determine if 'other' is a RECORD, a NODE, or something else.
     While *we* are only going to create sets where everything is a
     RECORD or everything is a NODE, the user can create any sort of
     set he/she wishes to create, so we need to handle that.  */
  if ((other->ob_type == &llines_event_type) ||
      (other->ob_type == &llines_other_type))
    {

      obj1 = _llpy_node_to_record (((LLINES_PY_NODE *)self)->lnn_node,
				   ((LLINES_PY_NODE *)self)->lnn_database);
      obj2 = _llpy_node_to_record (((LLINES_PY_NODE *)other)->lnn_node,
				   ((LLINES_PY_NODE *)other)->lnn_database);

      if (nztype (obj1) == nztype (obj2))
#if defined(DEADENDS)
	{
	  int keynum1 = atoi (&(obj1->root->key[1]));
	  int keynum2 = atoi (&(obj2->root->key[1]));

	  Py_RETURN_RICHCOMPARE (keynum1, keynum2, op);
	}
#else
	Py_RETURN_RICHCOMPARE (nzkeynum (obj1), nzkeynum (obj2), op);
#endif
      else
	Py_RETURN_RICHCOMPARE (nztype (obj1), nztype (obj2), op);
    }
  else if ((other->ob_type == &llines_family_type) ||
	   (other->ob_type == &llines_individual_type) ||
	   (other->ob_type == &llines_source_type))
    {
      switch (op)
	{
	case Py_LE:
	case Py_LT:
	case Py_EQ:
	  Py_RETURN_FALSE;

	case Py_GE:
	case Py_GT:
	case Py_NE:
	  Py_RETURN_TRUE;

	default:
	  Py_UNREACHABLE();
	}
    }
  else
    {
      switch (op)
	{
	case Py_LE:
	case Py_LT:
	case Py_NE:
	  Py_RETURN_TRUE;

	case Py_GE:
	case Py_GT:
	case Py_EQ:
	  Py_RETURN_FALSE;

	default:
	  Py_UNREACHABLE();
	}
    }
}

/* llines_record_hash -- we need a hash for the RECORD.  We would like
   the hash to be the same even if the RECORD gets flushed and read
   back in at a different location... (which I don't think can happen
   because we should be holding a reference to the RECORD..., but just
   in case...).  So, we use the RECORD's key.  And rather than design
   a new hash, we use Python's string hash.  */

Py_hash_t llines_record_hash (PyObject *obj)
{
  LLINES_PY_RECORD *pyobj = (LLINES_PY_RECORD *)obj;
  RECORD record;
  CNSTRING key;
  PyObject *hash_arg;
  Py_hash_t hash;

  if (! pyobj)
    {
      PyErr_SetString (PyExc_TypeError, "llines_record_hash: NULL argument");
      return -1;  /* raise an exception -- invalid argument */
    }

  record = pyobj->llr_record;
  if (! record)
    {
      PyErr_SetString (PyExc_TypeError, "llines_record_hash: not a RECORD");
      return -1;  /* raise an exception -- invalid argument */
    }

  key = nzkey (record);
  if (! key)
    {
      PyErr_SetString (PyExc_SystemError, "llines_record_hash: unable to determine key");
      return -1;	  /* raise an exception -- nzkey failed */
    }

  hash_arg = Py_BuildValue ("s", key);
  if (! hash_arg)
    return -1;			/* Py_BuildValue raised an exception */

  hash = PyObject_Hash (hash_arg);
  Py_DECREF (hash_arg);

  return (hash);
}

static void llpy_record_dealloc (PyObject *self)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_record_dealloc entry: self %p\n refcnt %ld",
	       (void *)self, Py_REFCNT (self));
    }
  release_record (fam->llr_record);
  fam->llr_record = 0;
  fam->llr_type = 0;
  Py_TYPE(self)->tp_free (self);
}

/* one time initialization related code */

PyTypeObject llines_record_type =
  {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "llines.Record",
    .tp_doc = "Lifelines GEDCOM generic Record",
    .tp_basicsize = sizeof (LLINES_PY_RECORD),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_dealloc = llpy_record_dealloc,
  };

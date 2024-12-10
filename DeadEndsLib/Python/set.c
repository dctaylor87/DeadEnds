/* set.c -- SET functions.

   These are the bulk of the functions that take and/or return a SET.

   While these functions return a SET, most (all?) allow any input
   that is iterable. */


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
#include "database.h"

#include "llpy-externs.h"

#include "python-to-c.h"
#include "types.h"
#include "py-set.h"		/* NEXT_FAMC */

/* forward references */

static PyObject *llpy_ancestorset (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_parentset (PyObject *self, PyObject *args, PyObject *kw);

/* helper routine for llpy_ancestorset and llpy_parentset */
static int add_parents (PyObject *obj, PyObject *working_set, PyObject *output_set);

static PyObject *llpy_siblingset (PyObject *self, PyObject *args, PyObject *kw);

/* start of code */

/* llpy_siblingset (SET,[include_self]) --> SET.  Produce as output
   the set of siblings of the input set.  If INCLUDE_SELF is True, the
   inputs are included in the output, otherwise they are not unless
   they are siblings of other INDIs in the input set.  */

static PyObject *llpy_siblingset (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "set", NULL };
  PyObject *set;	/* collection of input INDIs */
  PyObject *iterator;
  PyObject *py_indi;
  PyObject *output;
  int status;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "O", keywords, &set))
    return NULL;

  iterator = PyObject_GetIter (set);
  if (! iterator)
    return NULL;		/* non-iterable argument supplied */

  output = PySet_New (NULL);	/* output is an empty set */
  if (! output)
    {
      /* unable to create a new set, clean up and propagate the error */
      Py_DECREF (iterator);
      return NULL;
    }
  while ((py_indi = PyIter_Next (iterator)))
    {
      Database *database;
      RecordIndexEl *indi;
      GNode *node;

      if (py_indi->ob_type != &llines_individual_type)
	{
	  /* raise exception */
	}

      database = ((LLINES_PY_RECORD *)py_indi)->llr_database;
      indi = ((LLINES_PY_RECORD *)py_indi)->llr_record;

      /* for most individuals, the for loop stops after one iteration */
      for (GNode *famc = findTag (nztop (indi)->child, "FAMC");
	   famc;
	   famc = findTag (famc->sibling, "FAMC"))
	{
	  /* famc points to a family that indi is a child in */
	  node = keyToFamily (nval(famc), database->recordIndex); /* top node of family */

	  /* for each child of that family... */
	  for (GNode *child = findTag (node->child, "CHIL");
	       child;
	       child = findTag (child->sibling, "CHIL"))
	    {
	      CString child_key = nval (child);
	      if (nestr (child_key, nzkey (indi)))
		{
		  /* child in same family, different key, must be sibling */
		  RecordIndexEl *sibling = keyToPersonRecord (child_key, database);
		  LLINES_PY_RECORD *new_indi;

		  new_indi = PyObject_New (LLINES_PY_RECORD,
					   &llines_individual_type);
		  if (! new_indi)
		    {
		      /* should not happen -- out of memory? */
		      releaseRecord (sibling);

		      Py_DECREF (iterator);
		      PySet_Clear (output);
		      Py_DECREF (output);

		      return NULL;	/* PyObject_New set the exception */
		    }
		  new_indi->llr_type = LLINES_TYPE_INDI;
		  new_indi->llr_record = sibling;
		  new_indi->llr_database = database;
		  status = PySet_Add (output, (PyObject *)new_indi);
		  if (status < 0)
		    {
		      /* clean up and return */
		      Py_DECREF ((PyObject *)new_indi);

		      Py_DECREF (iterator);
		      PySet_Clear (output);
		      Py_DECREF (output);

		      return NULL;	/* PYSet_Add set the exception */
		    }
		}
	    }
	}
    }

  /* how did we get out of the while?  iterator exhaustion?  or exception? */
  if (PyErr_Occurred())
    {
      /* clean up and return */
      Py_DECREF (iterator);
      PySet_Clear (output);
      Py_DECREF (output);

      return NULL;
    }
  /* no errors and we are *DONE* */
  return (output);

}

/* llpy_ancestorset(SET) -- given an input set of INDIs, produce an
   output set of those INDIs that are the ancestors of the input
   INDIs.  */

static PyObject *llpy_ancestorset (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "set", NULL };
  PyObject *input_set;		/* set passed in */
  PyObject *working_set; /* represents INDIs that are not yet processed */
  PyObject *output_set;	/* represents INDIs that are part of the return value */
  PyObject *item;
  int status;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "O", keywords, &input_set))
    return NULL;

  output_set = PySet_New (NULL);
  if (! output_set)
    return NULL;

  working_set = PySet_New (NULL);
  if (! working_set)
    {
      Py_DECREF (output_set);
      return NULL;
    }

  /* propagate parents of the input set into working_set */
  if (llpy_debug)
    {
      fprintf (stderr, "llpy_ancestorset: processing input_set\n");
    }

  PyObject *iterator = PyObject_GetIter (input_set);
  if (! iterator)
    {
      Py_DECREF (working_set);
      Py_DECREF (output_set);
      return NULL;
    }
  while ((item = PyIter_Next (iterator)))
    {
      if ((status = add_parents (item, working_set, NULL)) < 0)
	{
	  /* report status, cleanup, return NULL */
	  PySet_Clear (working_set);
	  Py_DECREF (working_set);
	  PySet_Clear (output_set);
	  Py_DECREF (output_set);
	  Py_DECREF (iterator);
	  return NULL;
	}
    }
  if (PyErr_Occurred())
    {
      /* clean up and return NULL */
      PySet_Clear (working_set);
      Py_DECREF (working_set);
      PySet_Clear (output_set);
      Py_DECREF (output_set);
      Py_DECREF (iterator);
      return NULL;
    }
  Py_DECREF (iterator);

  /* go through working_set, putting parents into working_set, and
     item into output_set */
  if (llpy_debug)
    {
      fprintf (stderr, "llpy_ancestorset: processing working_set\n");
    }
  while ((PySet_GET_SIZE (working_set) > 0) && (item = PySet_Pop (working_set)))
    {
      int status = add_parents (item, working_set, output_set);
      if (status < 0)
	{
	  /* report status, cleanup, return NULL */
	  PySet_Clear (working_set);
	  Py_DECREF (working_set);
	  PySet_Clear (output_set);
	  Py_DECREF (output_set);
	  return NULL;
	}
    }
  ASSERT (PySet_GET_SIZE (working_set) == 0);
  PySet_Clear (working_set);	/* should be empty! */
  Py_DECREF (working_set);

  return (output_set);
}

/* llpy_parentset(SET) -- given an input set of INDIs, produce an
   output set of those INDIs that are the parents of the input
   INDIs.  */

static PyObject *llpy_parentset (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "set", NULL };
  PyObject *input_set;		/* set passed in */
  PyObject *output_set;	/* represents INDIs that are part of the return value */
  PyObject *item;
  int status;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "O", keywords, &input_set))
    return NULL;

  output_set = PySet_New (NULL);
  if (! output_set)
    return NULL;

  /* propagate parents of the input set into output_set */
  if (llpy_debug)
    {
      fprintf (stderr, "llpy_parentset: processing input_set\n");
    }

  PyObject *iterator = PyObject_GetIter (input_set);
  if (! iterator)
    {
      Py_DECREF (output_set);
      return NULL;
    }
  while ((item = PyIter_Next (iterator)))
    {
      if ((status = add_parents (item, output_set, NULL)) < 0)
	{
	  /* report status, cleanup, return NULL */
	  PySet_Clear (output_set);
	  Py_DECREF (output_set);
	  Py_DECREF (iterator);
	  return NULL;
	}
    }
  if (PyErr_Occurred())
    {
      /* clean up and return NULL */
      PySet_Clear (output_set);
      Py_DECREF (output_set);
      Py_DECREF (iterator);
      return NULL;
    }
  Py_DECREF (iterator);

  return (output_set);
}

/* add_parents -- for an individual 'indi', we first look to
   see if he/she is in output_set.  If yes, he/she was previously
   processed and we skip him/her, returning success.  If no, we add
   him/her to output_set and we look to see if he/she is a child in
   some family. If yes, we add HUSB and/or WIFE, if known, to
   working_set.   

   If output_set is NULL, it is skipped.  The output_set is NULL if
   this is the initial scan of the input set (ancestorset) or it is
   called from parentset.  */

static int add_parents (PyObject *obj, PyObject *working_set, PyObject *output_set)
{
  RecordIndexEl *indi = ((LLINES_PY_RECORD *)obj)->llr_record;
  Database *database = ((LLINES_PY_RECORD *)obj)->llr_database;
  GNode *indi_node = nztop (indi);
  GNode *famc = FAMC (indi_node);
  RecordIndexEl *fam;
  GNode *fam_node;
  GNode *parent;
  int status;

  if (output_set)
    {
      status = PySet_Contains (output_set, obj);
      if (status == 1)
	{
	  if (llpy_debug)
	    {
	      fprintf (stderr, "add_parents: INDI %s already present\n",
		       nzkey (((LLINES_PY_RECORD *)obj)->llr_record));
	    }
	  return 0;			/* already present, nothing to do, success */
	}
      else if (status < 0)
	return (status);		/* failure, let caller cleanup... */

      /* obj is not in output set *AND* just came (in caller) from working_set */
      if (llpy_debug)
	{
	  fprintf (stderr, "add_parents: adding INDI %s to output_set\n",
		   nzkey (((LLINES_PY_RECORD *)obj)->llr_record));
	}
      if (PySet_Add (output_set, obj) < 0)
	return (-8);
    }

  /* the individual is NOT present in output_set */

  if (! famc)
    {
      if (llpy_debug)
	{
	  fprintf (stderr, "add_parents: INDI %s has no known parents\n",
		   nzkey (((LLINES_PY_RECORD *)obj)->llr_record));
	}
      return 0;			/* no parents, nothing to do, success */
    }

  fam = keyToFamilyRecord (nval (famc), database);
  fam_node = nztop (fam);
  if ((parent = HUSB (fam_node)))
    {
      RecordIndexEl *record = keyToPersonRecord (nval (parent), database);
      LLINES_PY_RECORD *new_indi = PyObject_New (LLINES_PY_RECORD,
						      &llines_individual_type);
      if (! new_indi)
	return (-2);

      if (llpy_debug)
	{
	  fprintf (stderr, "add_parents: adding HUSB %s to working set\n",
		   nzkey (record));
	}
      new_indi->llr_type = LLINES_TYPE_INDI;
      new_indi->llr_record = record;
      if (PySet_Add (working_set, (PyObject *)new_indi) < 0)
	return (-3);
#if 0
      if (output_set)
	{
	  if (PySet_Add (output_set, (PyObject *)new_indi) < 0)
	    return (-4);
	}
#endif
    }
  if ((parent = WIFE (fam_node)))
    {
      RecordIndexEl *record = keyToPersonRecord (nval (parent), database);
      LLINES_PY_RECORD *new_indi = PyObject_New (LLINES_PY_RECORD,
						      &llines_individual_type);
      if (! new_indi)
	return (-5);

      if (llpy_debug)
	{
	  fprintf (stderr, "add_parents: adding WIFE %s to working set\n",
		   nzkey (record));
	}
      new_indi->llr_type = LLINES_TYPE_INDI;
      new_indi->llr_record = record;
      if (PySet_Add (working_set, (PyObject *)new_indi) < 0)
	return (-6);
#if 0
      if (output_set)
	{
	  if (PySet_Add (output_set, (PyObject *)new_indi) < 0)
	    return (-7);
	}
#endif
    }
  if (llpy_debug)
    {
      fprintf (stderr, "add_parents: successfully processed INDI %s\n",
	       nzkey (((LLINES_PY_RECORD *)obj)->llr_record));
    }
  return (0);
}

static struct PyMethodDef Lifelines_Set_Functions[] =
  {
   { "siblingset",	(PyCFunction)llpy_siblingset, METH_VARARGS | METH_KEYWORDS,
     "siblingset(set) --> SET of INDIs.\n\n\
Returns the set of all siblings of the INDIs in the input SET.\n\
NOTE: an INDI is NOT a sibling to himself/herself." },
   { "ancestorset",	(PyCFunction)llpy_ancestorset, METH_VARARGS | METH_KEYWORDS,
     "ancestorset(set) --> SET. Returns the set of ancestors of the input set." },
   { "parentset",	(PyCFunction)llpy_parentset, METH_VARARGS | METH_KEYWORDS,
     "parentset(set) --> SET.  Returns the parents of the input INDIs." },
   { NULL, 0, 0, NULL }		/* sentinel */
  };

void llpy_set_init (void)
{
  int status;

  status = PyModule_AddFunctions (Lifelines_Module, Lifelines_Set_Functions);

  if (status != 0)
    fprintf (stderr, "llpy_set_init: attempt to add functions returns %d\n", status);
}

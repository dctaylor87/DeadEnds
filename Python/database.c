/* database.c -- database functions

   These are functions that are specific to the database.  They
   include functions that return a person or a family or other type of
   record that are not methods of the type -- e.g., firstindi,
   lastindi -- and functions that are specific to the database --
   e.g., the pathname */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */

#if defined(DEADENDS)
#include "porting.h"		/* LifeLines --> DeadEnds */
#include "standard.h"		/* String */
#include "gnode.h"		/* GNode */
#include "recordindex.h"	/* RecordIndexEl */
#else
#include "standard.h"		/* STRING */
#include "llstdlib.h"		/* CALLBACK_FNC */
#include "lloptions.h"

#include "gedcom.h"		/* RECORD */
#endif

#include "python-to-c.h"
#include "types.h"
#include "database.h"

#if !defined(DEADENDS)		/* XXX maybe someday... XXX */
/* forward references */

/* return the first individual in the database (in keynum order) */
static PyObject *llpy_firstindi (PyObject *self, PyObject *args);

/* return the last individual in the database (in keynum order) */
static PyObject *llpy_lastindi (PyObject *self, PyObject *args);

/* return the first family in the database (in keynum order) */
static PyObject *llpy_firstfam (PyObject *self, PyObject *args);

/* return the last family in the database (in keynum order) */
static PyObject *llpy_lastfam (PyObject *self, PyObject *args);

 /* llpy_firstindi (void) --> INDI

   Returns the first INDI in the database in key order.  */

static PyObject *llpy_firstindi (PyObject *Py_UNUSED(self), PyObject *args ATTRIBUTE_UNUSED)
{
  INT keynum = xref_firsti();
  LLINES_PY_RECORD *rec;

  if (keynum == 0)
    Py_RETURN_NONE;		/* no individuals in the database */

  rec = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! rec)
    return NULL;

  rec->llr_type = LLINES_TYPE_INDI;
  rec->llr_record = keynum_to_irecord (keynum);
  return (PyObject *)rec;
}

/* llpy_lastindi (void) --> INDI

   Returns the last INDI in the database in key order.  */

static PyObject *llpy_lastindi (PyObject *Py_UNUSED(self), PyObject *args ATTRIBUTE_UNUSED)
{
  INT keynum = xref_lasti();
  LLINES_PY_RECORD *rec;

  if (keynum == 0)
    Py_RETURN_NONE;		/* no individuals in the database */

  rec = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! rec)
    return NULL;

  rec->llr_type = LLINES_TYPE_INDI;
  rec->llr_record = keynum_to_irecord (keynum);
  return (PyObject *)rec;
}

/* llpy_firstfam (void) --> FAM

   Returns the first FAM in the database in key order.  */

static PyObject *llpy_firstfam (PyObject *Py_UNUSED(self), PyObject *args ATTRIBUTE_UNUSED)
{
  INT keynum = xref_firstf();
  LLINES_PY_RECORD *rec;

  if (keynum == 0)
    Py_RETURN_NONE;		/* no families in the database */

  rec = PyObject_New (LLINES_PY_RECORD, &llines_family_type);
  if (! rec)
    return NULL;

  rec->llr_type = LLINES_TYPE_FAM;
  rec->llr_record = keynum_to_frecord (keynum);
  return (PyObject *)rec;
}

/* llpy_lastfam (void) --> FAM

   Returns the last FAM in the database in key order.  */

static PyObject *llpy_lastfam (PyObject *Py_UNUSED(self), PyObject *args ATTRIBUTE_UNUSED)
{
  INT keynum = xref_lastf();
  LLINES_PY_RECORD *rec;

  if (keynum == 0)
    Py_RETURN_NONE;		/* no families in the database */

  rec = PyObject_New (LLINES_PY_RECORD, &llines_family_type);
  if (! rec)
    return NULL;

  rec->llr_type = LLINES_TYPE_FAM;
  rec->llr_record = keynum_to_frecord (keynum);
  return (PyObject *)rec;
}
#endif	/* XXX */

static struct PyMethodDef Lifelines_Database_Functions[] =
  {
#if !defined(DEADENDS)		/* XXX maybe someday... XXX */
   { "firstindi",	llpy_firstindi, METH_NOARGS,
     "firstindi(void) -> INDI: first individual in database (in key order)" },
   { "lastindi",	llpy_lastindi, METH_NOARGS,
     "lastindi(void) -> INDI: last individual in database  (in key order)" },
   { "firstfam",	llpy_firstfam, METH_NOARGS,
     "firstfam(void) -> FAM: first family in database (in key order)" },
   { "lastfam",		llpy_lastfam, METH_NOARGS,
     "lastfam(void) -> FAM: last family in database  (in key order)" },
#endif

   { NULL, 0, 0, NULL }		/* sentinel */
  };

PyTypeObject llines_database_type =
  {
   PyVarObject_HEAD_INIT(NULL,0)
   .tp_name = "llines.Database",
   .tp_doc = "Lifelines GEDCOM Database",
   .tp_basicsize = sizeof (LLINES_PY_DATABASE),
   .tp_itemsize = 0,
   .tp_flags = Py_TPFLAGS_DEFAULT,
   .tp_new = PyType_GenericNew,

   /* XXX for now -- someday the functions in this file will be
      instance functions XXX */
   .tp_methods = 0,
  };

void llpy_database_init (void)
{
  int status;

  status = PyModule_AddFunctions (Lifelines_Module, Lifelines_Database_Functions);
  if (status != 0)
    fprintf (stderr, "llpy_database_init: attempt to add functions returned %d\n", status);
}

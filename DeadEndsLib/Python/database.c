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
#include <stdint.h>

#include "porting.h"		/* LifeLines --> DeadEnds */
#include "standard.h"		/* String */
#include "denls.h"
#include "gedcom.h"
#include "hashtable.h"
#include "refnindex.h"
#include "gnode.h"		/* GNode */

#include "xref.h"
#include "python-to-c.h"
#include "types.h"
#include "database.h"

/* forward references */

/* return the first individual in the database (in keynum order) */
static PyObject *llpy_firstindi_db (PyObject *self, PyObject *args);

/* return the last individual in the database (in keynum order) */
static PyObject *llpy_lastindi_db (PyObject *self, PyObject *args);

/* return the first family in the database (in keynum order) */
static PyObject *llpy_firstfam_db (PyObject *self, PyObject *args);

/* return the last family in the database (in keynum order) */
static PyObject *llpy_lastfam_db (PyObject *self, PyObject *args);

/* these are database instance method iterators, for database function
   iterators, see iter.c */

static PyObject *llpy_events_db (PyObject *self, PyObject *args);
static PyObject *llpy_individuals_db (PyObject *self, PyObject *args);
static PyObject *llpy_families_db (PyObject *self, PyObject *args);
static PyObject *llpy_sources_db (PyObject *self, PyObject *args);
static PyObject *llpy_others_db (PyObject *self, PyObject *args);

 /* llpy_firstindi (void) --> INDI

   Returns the first INDI in the database in key order.  */

static PyObject *llpy_firstindi_db (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  Database *database = ((LLINES_PY_DATABASE *)self)->lld_database;
  GNode *new;
  LLINES_PY_RECORD *rec;

  if (! database)
    {
      PyErr_SetString (PyExc_SystemError, "firstindi: database object has NULL database");
      return NULL;
    }
  new = getFirstPersonRecord (database);
  if (! new)
    Py_RETURN_NONE;		/* no individuals in the database */

  rec = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! rec)
    return NULL;

  rec->llr_type = LLINES_TYPE_INDI;
  rec->llr_record = new;
  rec->llr_database = database;

  return (PyObject *)rec;
}

/* llpy_lastindi (void) --> INDI

   Returns the last INDI in the database in key order.  */

static PyObject *llpy_lastindi_db (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  Database *database = ((LLINES_PY_DATABASE *)self)->lld_database;
  GNode *new;
  LLINES_PY_RECORD *rec;

  if (! database)
    {
      PyErr_SetString (PyExc_SystemError, "firstindi: database object has NULL database");
      return NULL;
    }

  new = getLastPersonRecord (database);
  if (! new)
    Py_RETURN_NONE;		/* no individuals in the database */

  rec = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! rec)
    return NULL;

  rec->llr_type = LLINES_TYPE_INDI;
  rec->llr_record = new;
  rec->llr_database = database;

  return (PyObject *)rec;
}

/* llpy_firstfam (void) --> FAM

   Returns the first FAM in the database in key order.  */

static PyObject *llpy_firstfam_db (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  Database *database = ((LLINES_PY_DATABASE *)self)->lld_database;
  GNode *new;
  LLINES_PY_RECORD *rec;

  if (! database)
    {
      PyErr_SetString (PyExc_SystemError, "firstfam: database object has NULL database");
      return NULL;
    }

  new = getFirstFamilyRecord (database);
  if (! new)
    Py_RETURN_NONE;		/* no families in the database */

  rec = PyObject_New (LLINES_PY_RECORD, &llines_family_type);
  if (! rec)
    return NULL;

  rec->llr_type = LLINES_TYPE_FAM;
  rec->llr_record = new;
  rec->llr_database = database;

  return (PyObject *)rec;
}

/* llpy_lastfam (void) --> FAM

   Returns the last FAM in the database in key order.  */

static PyObject *llpy_lastfam_db (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  Database *database = ((LLINES_PY_DATABASE *)self)->lld_database;
  GNode *new;
  LLINES_PY_RECORD *rec;

  if (! database)
    {
      PyErr_SetString (PyExc_SystemError, "firstfam: database object has NULL database");
      return NULL;
    }

  new = getLastFamilyRecord (database);
  if (! new)
    Py_RETURN_NONE;		/* no families in the database */

  rec = PyObject_New (LLINES_PY_RECORD, &llines_family_type);
  if (! rec)
    return NULL;

  rec->llr_type = LLINES_TYPE_FAM;
  rec->llr_record = new;
  rec->llr_database = database;

  return (PyObject *)rec;
}

/* llpy_events (void) --> Returns an iterator for the set of
   events in the database.  */

static PyObject *llpy_events_db (PyObject *self,
				 PyObject *args ATTRIBUTE_UNUSED)
{
  Database *database = ((LLINES_PY_RECORD *)self)->llr_database;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_events entry: self %p args %p\n",
	       (void *)self, (void *)args);
    }

  return _llpy_create_record_iterator (database, LLINES_TYPE_EVEN);
}

/* llpy_individuals (void) --> Returns an iterator for the set of
   individuals in the database.  */

static PyObject *llpy_individuals_db (PyObject *self,
				      PyObject *args ATTRIBUTE_UNUSED)
{
  Database *database = ((LLINES_PY_RECORD *)self)->llr_database;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_individuals entry: self %p args %p\n",
	       (void *)self, (void *)args);
    }

  return _llpy_create_record_iterator (database, LLINES_TYPE_INDI);
}

/* llpy_families (void) --> Returns an iterator for the set of
   families in the database.  */

static PyObject *llpy_families_db (PyObject *self,
				   PyObject *args ATTRIBUTE_UNUSED)
{
  Database *database = ((LLINES_PY_RECORD *)self)->llr_database;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_families entry: self %p args %p\n",
	       (void *)self, (void *)args);
    }

  return _llpy_create_record_iterator (database, LLINES_TYPE_FAM);
}

/* llpy_sources (void) --> Returns an iterator for the set of sources
   in the database.  */

static PyObject *llpy_sources_db (PyObject *self,
				  PyObject *args ATTRIBUTE_UNUSED)
{
  Database *database = ((LLINES_PY_RECORD *)self)->llr_database;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_sources entry: self %p args %p\n",
	       (void *)self, (void *)args);
    }

  return _llpy_create_record_iterator (database, LLINES_TYPE_SOUR);
}

/* llpy_others (void) --> Returns an iterator for the set of other records
   in the database.  */

static PyObject *llpy_others_db (PyObject *self,
				 PyObject *args ATTRIBUTE_UNUSED)
{
  Database *database = ((LLINES_PY_RECORD *)self)->llr_database;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_others entry: self %p args %p\n",
	       (void *)self, (void *)args);
    }

  return _llpy_create_record_iterator (database, LLINES_TYPE_OTHR);
}

/* _llpy_create_record_iterator -- helper functiion used by all 10
   record iterators */

PyObject *_llpy_create_record_iterator (Database *database, int record_type)
{
  LLINES_PY_RECORD_ITER *iter = PyObject_New (LLINES_PY_RECORD_ITER,
					      &llines_record_iter_type);

  if (llpy_debug)
    {
      fprintf (stderr, "_llpy_create_record_iterator: database %p record_type %d\n",
	       (void *)database, record_type);
    }

  if (! iter)
    return NULL;		/* PyObject_New failed and set exception */

  iter->li_type = record_type;
  iter->li_database = database;
  iter->li_bucket_ndx = -1;
  iter->li_element_ndx = -1;
  return (PyObject *)iter;
}

static PyObject *
llpy_export_db (PyObject *self, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "file", "version", "submitter", NULL };
  char *filename = 0;
  char *gedcom_version = 0;
  char *submitter = 0;
  Database *database = ((LLINES_PY_DATABASE *)self)->lld_database;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "s|zzO!", keywords,
				     &filename, &gedcom_version, &submitter))
    return NULL;

  return _llpy_export (database, filename, gedcom_version, submitter);
}

static struct PyMethodDef Lifelines_Database_Functions[] =
  {
#if 0
   { "firstindi",	llpy_firstindi, METH_NOARGS,
     "firstindi(void) -> INDI: first individual in database (in key order)" },
   { "lastindi",	llpy_lastindi, METH_NOARGS,
     "lastindi(void) -> INDI: last individual in database  (in key order)" },
   { "firstfam",	llpy_firstfam, METH_NOARGS,
     "firstfam(void) -> FAM: first family in database (in key order)" },
   { "lastfam",		llpy_lastfam, METH_NOARGS,
     "lastfam(void) -> FAM: last family in database  (in key order)" },
#endif
#if 0
#if defined(DEADENDS)
   { "get_database",	(PyCFunction)llpy_current_database, METH_VARARGS | METH_KEYWORDS,
     "get_database([name=STRING) --> DATABASE: Returns the specified database.\n\n\
If NAME is omitted or is the empty string, the current database is returned." },
#endif
#endif
   { NULL, 0, 0, NULL }		/* sentinel */
  };

static struct PyMethodDef Lifelines_Database_Methods[] =
  {
    /* DATABASE method variants of the iterator functions */

   { "individuals",	(PyCFunction)llpy_individuals_db, METH_VARARGS | METH_KEYWORDS,
     "(DATABASE).individuals(void) -> iterator for the set of all INDI in the database" },
   { "families",	(PyCFunction)llpy_families_db, METH_VARARGS | METH_KEYWORDS,
     "(DATABASE).families(void) -> iterator for the set of all FAM in the database" },
   { "sources",		(PyCFunction)llpy_sources_db, METH_VARARGS | METH_KEYWORDS,
     "(DATABASE).sources(void) -> iterator for the set of all SOUR in the database" },
   { "events",		(PyCFunction)llpy_events_db, METH_VARARGS | METH_KEYWORDS,
     "(DATABASE).events(void) --> iterator for the set of all EVEN in the database" },
   { "others",		(PyCFunction)llpy_others_db, METH_VARARGS | METH_KEYWORDS,
     "(DATABASE).others(void) --> iterator for the set of all OTHR records in the database" },

   { "export",		(PyCFunction)llpy_export_db, METH_VARARGS | METH_KEYWORDS,
     "(DATABASE).export(file, [version], [submitter]) --> None or error" },

   { "firstindi",	(PyCFunction)llpy_firstindi_db, METH_NOARGS,
     "(DATABASE).firstindi(void) -> INDI: first individual in database (in key order)" },
   { "lastindi",	llpy_lastindi_db, METH_NOARGS,
     "(DATABASE).lastindi(void) -> INDI: last individual in database  (in key order)" },
   { "firstfam",	(PyCFunction)llpy_firstfam_db, METH_NOARGS,
     "(DATABASE).firstfam(void) -> FAM: first family in database (in key order)" },
   { "lastfam",		llpy_lastfam_db, METH_NOARGS,
     "(DATABASE).lastfam(void) -> FAM: last family in database  (in key order)" },
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
   .tp_methods = Lifelines_Database_Methods,
  };

void llpy_database_init (void)
{
  int status;

  status = PyModule_AddFunctions (Lifelines_Module, Lifelines_Database_Functions);
  if (status != 0)
    fprintf (stderr, "llpy_database_init: attempt to add functions returned %d\n", status);
}

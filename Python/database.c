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

#if defined(DEADENDS)
#include "porting.h"		/* LifeLines --> DeadEnds */
#include "standard.h"		/* String */
#include "llnls.h"
#include "refnindex.h"
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

/* these are database instance method iterators, for database function
   iterators, see iter.c */

static PyObject *llpy_events_db (PyObject *self, PyObject *args);
static PyObject *llpy_individuals_db (PyObject *self, PyObject *args);
static PyObject *llpy_families_db (PyObject *self, PyObject *args);
static PyObject *llpy_sources_db (PyObject *self, PyObject *args);
static PyObject *llpy_others_db (PyObject *self, PyObject *args);

 /* llpy_firstindi (void) --> INDI

   Returns the first INDI in the database in key order.  */

static PyObject *llpy_firstindi (PyObject *Py_UNUSED(self), PyObject *args ATTRIBUTE_UNUSED)
{
  int keynum = xref_firsti();
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
  int keynum = xref_lasti();
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
  int keynum = xref_firstf();
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
  int keynum = xref_lastf();
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
#if defined(DEADENDS)
  iter->li_bucket_ndx = -1;
  iter->li_element_ndx = -1;
#else
  iter->li_current = 0;
#endif
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
     "individuals(void) -> iterator for the set of all INDI in the database" },
   { "families",	(PyCFunction)llpy_families_db, METH_VARARGS | METH_KEYWORDS,
     "families(void) -> iterator for the set of all FAM in the database" },
   { "sources",		(PyCFunction)llpy_sources_db, METH_VARARGS | METH_KEYWORDS,
     "sources(void) -> iterator for the set of all SOUR in the database" },
   { "events",		(PyCFunction)llpy_events_db, METH_VARARGS | METH_KEYWORDS,
     "events(void) --> iterator for the set of all EVEN in the database" },
   { "others",		(PyCFunction)llpy_others_db, METH_VARARGS | METH_KEYWORDS,
     "others(void) --> iterator for the set of all OTHR records in the database" },

   { "export",		(PyCFunction)llpy_export_db, METH_VARARGS | METH_KEYWORDS,
     "export(file, [version], [submitter]) --> None or error" },

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

/* python-to-c.c -- this module provides the glue for Python code to call into Lifelines C routines */

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

#include "llstdlib.h"
#include "gedcom.h"
#include "../gedlib/leaksi.h"
#include "version.h"		/* get_deadends_version */
#endif

#include "python-to-c.h"
#include "llpy-externs.h"

#include "database.h"
#include "family.h"
#include "person.h"
#include "types.h"

/* variables defined here */

PyObject *Lifelines_Module;

int llpy_debug = 0;	      /* XXX TODO: make this ia bitmask XXX */

/* forward references */

static PyObject *llpy_version (PyObject *self, PyObject *args);

/* start of code */

static PyObject *llpy_version (PyObject *self ATTRIBUTE_UNUSED, PyObject *args ATTRIBUTE_UNUSED)
{
#if defined(DEADENDS)
  return (Py_BuildValue ("s", version));
#else
  return (Py_BuildValue ("s", get_lifelines_version(100)));
#endif
}

PyObject *_llpy_key (PyObject *self, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "strip_prefix", NULL };
  LLINES_PY_RECORD *record = (LLINES_PY_RECORD *)self;
  int strip_prefix = 0;		/* by default, leave the prefix on */
  CString key;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|p", keywords, &strip_prefix))
    return NULL;

  key = nzkey(record->llr_record);
  /* currently the prefix is always a single character.  if that
     changes, this needs modification */
  return Py_BuildValue ("s", (strip_prefix ? key + 1 : key));
}

PyObject *_llpy_top_node (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *py_record = (LLINES_PY_RECORD *) self;
  RECORD record = py_record->llr_record;
  NODE top = nztop (record);
  LLINES_PY_NODE *py_node;

  if (! top)
    {
      /* something went wrong -- cannot find the top node for this record */
      PyErr_SetString(PyExc_SystemError, "top_node: unable to find RECORD's top NODE");
      return NULL;
    }
  py_node = PyObject_New (LLINES_PY_NODE, &llines_node_type);
  if (! py_node)
    return NULL;

  py_node->lnn_type = py_record->llr_type;
  nrefcnt_inc(top);
  TRACK_NODE_REFCNT_INC(top);
  py_node->lnn_database = py_record->llr_database;
  py_node->lnn_node = top;
  return ((PyObject *) py_node);
}

static struct PyMethodDef LifelinesMethods[] =
  {
   /* Person Functions -- moved to Lifelines_Person_Methods */

   /* GEDCOM Node Functions -- now in nodes.c */

   /* Event and Date Functions */

#if 0				/* XXX need serious thought due to existing implementation XXX */

   { "gettoday",	llpy_gettoday, METH_NOARGS,
     "doc string" },
   { "setdate",		llpy_setdate, METH_VARARGS,
     "doc string" },
   { "datepic",		llpy_datepic, METH_VARARGS,
     "datepic(STRING) --> VOID: set custom date format for stddate calls (what about errors?)." },
   { "complexformat",	llpy_complexformat, METH_VARARGS,
     "doc string" },
   { "complexpic",	llpy_complexpic, METH_VARARGS,
     "doc string" },
   { "complexdate",	llpy_complexdate, METH_VARARGS,
     "doc string" },
   { "dayofweek",	llpy_dayofweek, METH_VARARGS,
     "doc string" },
   { "date2jd",		llpy_date2jd, METH_VARARGS,
     "doc string" },
   { "jd2date",		llpy_jd2date, METH_VARARGS,
     "doc string" },
#endif

   /* Value Extraction Functions */
#if 0
   { "extractdate",	llpy_extractdate, METH_VARARGS,
     "doc string" },
   { "extractnames",	llpy_extractnames, METH_VARARGS,
     "doc string" },
   { "extractplaces",	llpy_extractplaces, METH_VARARGS,
     "doc string" },

   { "extractdatestr",	llpy_extractdatestr, METH_VARARGS,
     "doc string" },
#endif

   /* User Interaction Functions */

#if 0				/* XXX until we figure out what to do about sets XXX */
   { "getindiset",	llpy_getindiset, METH_VARARGS,
     "identify set of persons through user interface" },
#endif
#if 0
   { "chooseindi",	llpy_chooseindi, METH_VARARGS,
     "doc string" },
   { "choosesubset",	llpy_choosesubset, METH_VARARGS,
     "doc string" },
#endif

   /* Person Set Functions and GEDCOM Extraction */

#if 0				/* XXX until we figure out what to do about sets XXX */
   { "indiset",		llpy_indiset, METH_VARARGS,
     "doc string" },

   { "gengedcomstrong",	llpy_gengedcomstrong, METH_VARARGS,
     "doc string" },
#endif

   /* Miscellaneous Functions */

   { "version",		llpy_version, METH_NOARGS,
     "version(void) -> STRING: return version of current program" },

   { NULL, 0, 0, NULL }		/* sentinel */
};

static PyModuleDef lifelines_module =
  {
   PyModuleDef_HEAD_INIT,
   .m_name = "llines",
   .m_doc = "Lifelines extension module",
   .m_size = -1,
   .m_methods = LifelinesMethods,
  };

static PyMODINIT_FUNC
PyInit_llines(void)
{
  PyObject *module;

  if (PyType_Ready(&llines_node_type) < 0)
    return NULL;

  if (PyType_Ready(&llines_family_type) < 0)
    return NULL;

  if (PyType_Ready(&llines_individual_type) < 0)
    return NULL;

  if (PyType_Ready(&llines_source_type) < 0)
    return NULL;

  if (PyType_Ready(&llines_other_type) < 0)
    return NULL;

  if (PyType_Ready(&llines_record_type) < 0)
    return NULL;

  if (PyType_Ready(&llines_database_type) < 0)
    return NULL;

  if (PyType_Ready(&llines_record_iter_type) < 0)
    return NULL;

  if (PyType_Ready(&llines_node_iter_type) < 0)
    return NULL;

  if (PyType_Ready(&llines_event_type) < 0)
    return NULL;

  module = PyModule_Create (&lifelines_module);
  if (module == NULL)
    return NULL;

  Lifelines_Module = module;
  Py_INCREF (Lifelines_Module);	/* not sure if this is needed... */

  Py_INCREF (&llines_node_type);
  if (PyModule_AddObject (module, "Node", (PyObject *) &llines_node_type) < 0)
    {
      Py_DECREF (&llines_node_type);
      Py_DECREF (module);
      return NULL;
    }

  Py_INCREF (&llines_family_type);
  if (PyModule_AddObject (module, "Family", (PyObject *) &llines_family_type) < 0)
    {
      Py_DECREF (&llines_family_type);
      Py_DECREF (&llines_node_type);
      Py_DECREF (module);
      return NULL;
    }

  Py_INCREF (&llines_individual_type);
  if (PyModule_AddObject (module, "Individual", (PyObject *) &llines_individual_type) < 0)
    {
      Py_DECREF (&llines_individual_type);
      Py_DECREF (&llines_family_type);
      Py_DECREF (&llines_node_type);
      Py_DECREF (module);
      return NULL;
    }

  Py_INCREF (&llines_event_type);
  if (PyModule_AddObject (module, "Event", (PyObject *) &llines_event_type) < 0)
    {
      Py_DECREF (&llines_event_type);
      Py_DECREF (&llines_individual_type);
      Py_DECREF (&llines_family_type);
      Py_DECREF (&llines_node_type);
      Py_DECREF (module);
      return NULL;
    }

  Py_INCREF (&llines_source_type);
  if (PyModule_AddObject (module, "Source", (PyObject *) &llines_source_type) < 0)
    {
      Py_DECREF (&llines_source_type);
      Py_DECREF (&llines_event_type);
      Py_DECREF (&llines_individual_type);
      Py_DECREF (&llines_family_type);
      Py_DECREF (&llines_node_type);
      Py_DECREF (module);
      return NULL;
    }

  Py_INCREF (&llines_other_type);
  if (PyModule_AddObject (module, "Other", (PyObject *) &llines_other_type) < 0)
    {
      Py_DECREF (&llines_other_type);
      Py_DECREF (&llines_source_type);
      Py_DECREF (&llines_event_type);
      Py_DECREF (&llines_individual_type);
      Py_DECREF (&llines_family_type);
      Py_DECREF (&llines_node_type);
      Py_DECREF (module);
      return NULL;
    }

  Py_INCREF (&llines_record_type);
  if (PyModule_AddObject (module, "Record", (PyObject *) &llines_record_type) < 0)
    {
      Py_DECREF (&llines_record_type);
      Py_DECREF (&llines_other_type);
      Py_DECREF (&llines_source_type);
      Py_DECREF (&llines_event_type);
      Py_DECREF (&llines_individual_type);
      Py_DECREF (&llines_family_type);
      Py_DECREF (&llines_node_type);
      Py_DECREF (module);
      return NULL;
    }

  Py_INCREF (&llines_record_iter_type);
  if (PyModule_AddObject (module, "Iter", (PyObject *) &llines_record_iter_type) < 0)
    {
      Py_DECREF (&llines_record_iter_type);
      Py_DECREF (&llines_record_type);
      Py_DECREF (&llines_other_type);
      Py_DECREF (&llines_source_type);
      Py_DECREF (&llines_event_type);
      Py_DECREF (&llines_individual_type);
      Py_DECREF (&llines_family_type);
      Py_DECREF (&llines_node_type);
      Py_DECREF (module);
      return NULL;
    }

  Py_INCREF (&llines_node_iter_type);
  if (PyModule_AddObject (module, "Iter", (PyObject *) &llines_node_iter_type) < 0)
    {
      Py_DECREF (&llines_node_iter_type);
      Py_DECREF (&llines_record_iter_type);
      Py_DECREF (&llines_record_type);
      Py_DECREF (&llines_other_type);
      Py_DECREF (&llines_source_type);
      Py_DECREF (&llines_event_type);
      Py_DECREF (&llines_individual_type);
      Py_DECREF (&llines_family_type);
      Py_DECREF (&llines_node_type);
      Py_DECREF (module);
      return NULL;
    }

  Py_INCREF (&llines_database_type);
  if (PyModule_AddObject (module, "Database", (PyObject *) &llines_database_type) < 0)
    {
      Py_DECREF (&llines_database_type);
      Py_DECREF (&llines_node_iter_type);
      Py_DECREF (&llines_record_iter_type);
      Py_DECREF (&llines_record_type);
      Py_DECREF (&llines_other_type);
      Py_DECREF (&llines_source_type);
      Py_DECREF (&llines_event_type);
      Py_DECREF (&llines_individual_type);
      Py_DECREF (&llines_family_type);
      Py_DECREF (&llines_node_type);
      Py_DECREF (module);
      return NULL;
    }

  llpy_user_init ();
  llpy_set_init ();
  llpy_iter_init ();
  llpy_database_init ();
  llpy_nodes_init ();
  llpy_records_init ();
  llpy_event_init ();
  llpy_person_init ();
  llpy_export_init ();

  return (module);
}

/* when Python support is compiled in, this routine must be called
   during the llines initialization phase. */

void llpy_init (void)
{
  PyImport_AppendInittab ("llines", &PyInit_llines);
}

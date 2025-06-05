/* list.c -- functions, not methods, that produce a list as output */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */

#include "porting.h"		/* LifeLines --> DeadEnds */
#include "standard.h"		/* String */
#include "gnode.h"
#include "recordindex.h"
#include "database.h"
#include "nameindex.h"
#include "denls.h"

#include "python-to-c.h"
#include "list.h"
#include "set.h"
#include "types.h"

static PyObject *llpy_name_to_keylist (PyObject *self ATTRIBUTE_UNUSED,
				    PyObject *args,
				    PyObject *kw)
{
  char *name = NULL;
  PyObject *object;
  Database *database;
  Py_ssize_t len = 0;
  PyObject *py_list;
  static char *keywords[] = { "name", "database", NULL };
  Set *set;
  int ndx;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "s|O", keywords, &name, &object))
    return NULL;

  if (object)
    {
      if (Py_TYPE (object) == &llines_database_type)
	database = ((LLINES_PY_DATABASE *)object)->lld_database;
      else
	{
	  PyErr_SetString (PyExc_TypeError, "name_to_keylist: database argument must be a database object");
	  return NULL;
	}
    }
  else
    database = currentDatabase;

  set = searchNameIndex (database->nameIndex, name);

  if (! set || ((len = lengthSet(set)) == 0))
    Py_RETURN_NONE;

  py_list = PyList_New (len);
  if (! py_list)
    return NULL;

  ndx = 0;
  FORSET(set, element)
    PyObject *object = Py_BuildValue ("s", element);
    if (! object)
      {
	for (int ndx2 = 0; ndx2 < ndx; ndx2++)
	  ;			/* XXX insert code to free elements of list XXX */
	return NULL;
      }
    PyList_SET_ITEM (py_list, ndx, object);
    ndx++;
  ENDSET
    return py_list;
}

static struct PyMethodDef Lifelines_List_Functions[] =
  {
    { "name_to_keylist",	(PyCFunction)llpy_name_to_keylist, METH_VARARGS | METH_KEYWORDS,
      "name_to_keylist(name, [database]) --> list of keys of matching database entries\n\n\
NAME is pattern to search.\n						\
DATABASE is which database to search, current database by default." },
    { NULL, 0, 0, NULL }		/* sentinel */
  };

/* XXX rethink name of name_to_keylist function AND add a database
   method version.  XXX */

void llpy_list_init (void)
{
  int status;

  status = PyModule_AddFunctions (Lifelines_Module, Lifelines_List_Functions);
  if (status != 0)
    fprintf (stderr, "llpy_list_init: attempt to add functions returned %d\n", status);
}

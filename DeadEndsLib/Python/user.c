/* user.c -- user interaction */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdint.h>
#include <sys/param.h>		/* MAXPATHLEN */

#include "porting.h"		/* LifeLines --> DeadEnds */
#include "standard.h"		/* String */
#include "database.h"
#include "sequence.h"

#include "denls.h"
#include "hashtable.h"
#include "refnindex.h"
#include "gnode.h"		/* GNode */
#include "errors.h"
#include "ask.h"
#include "rptui.h"
#include "py-messages.h"
#include "feedback.h"

#include "python-to-c.h"
#include "types.h"

static PyObject *llpy_getindi (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_getfam (PyObject *self, PyObject *args , PyObject *kw);
static PyObject *llpy_getint (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_getstr (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_menuchoose (PyObject *self, PyObject *args, PyObject *kw);

/* llpy_getindi (PROMPT) --> INDI: identify person through user interface */

static PyObject *llpy_getindi (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  String prompt = _("Identify person for program:");
  GNode *rec;
  LLINES_PY_DATABASE *py_database = 0;
  Database *database;
  static char *keywords[] = { "prompt", "database", NULL };
  LLINES_PY_RECORD *indi;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|sO!", keywords, &prompt,
				     &llines_database_type, &py_database))
    return NULL;

  if (py_database)
    database = py_database->lld_database;
  else
    database = currentDatabase;

  rec = rptui_ask_for_indi (prompt, DOASK1, database);

  indi = PyObject_New(LLINES_PY_RECORD, &llines_individual_type);
  indi->llr_type = LLINES_TYPE_INDI;
  indi->llr_record = rec;
  indi->llr_database = database;
  return (PyObject *)indi;
}

/* llpy_getfam (void) --> FAM; Identify family through user interface */

static PyObject *llpy_getfam (PyObject *self ATTRIBUTE_UNUSED, PyObject *args , PyObject *kw){

  LLINES_PY_RECORD *family;
  GNode *record;
  LLINES_PY_DATABASE *py_database = 0;
  Database *database;
  static char *keywords[] = { "database", NULL };

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|O!", keywords,
				     &llines_database_type, &py_database))
    return NULL;

  if (py_database)
    database = py_database->lld_database;
  else
    database = currentDatabase;

  record = rptui_ask_for_fam(_("Enter a spouse from family."),
			     _("Enter a sibling from family."), database);
  if (! record)
    Py_RETURN_NONE;		/* user cancelled */

  family = PyObject_New (LLINES_PY_RECORD, &llines_family_type);
  if (! family)
    return NULL;		/* PyObject_New failed and set the exception */

  family->llr_record = record;
  family->llr_type = LLINES_TYPE_FAM;
  family->llr_database = database;
  return (PyObject *)family;
}

/* llpy_getint ([prompt]) --> INT; Get integer through user interface. */

static PyObject *llpy_getint (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  String prompt = _("Enter integer for program");
  static char *keywords[] = { "prompt", NULL };
  int num;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|s", keywords, &prompt))
    return NULL;

  if (! rptui_ask_for_int (prompt, &num))
    {
      Py_RETURN_NONE;		/* user cancelled */
    }
  return Py_BuildValue ("i", num);
}

/* llpy_getstr ([prompt]) --> String; Get string through user interface. */

static PyObject *llpy_getstr (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "prompt", NULL };
  String prompt = _(qSchoostrttl);
  char buffer[MAXPATHLEN];

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|s", keywords, &prompt))
    return NULL;

  if (! ask_for_string (prompt, _(qSaskstr), buffer, sizeof(buffer)))
    /* cancelled -- return empty string */
    buffer[0] = 0;

  return Py_BuildValue ("s", buffer);
}

/* llpy_getindiset ([prompt], [database]) --> SET */

static PyObject *llpy_getindiset (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "prompt", "database", NULL };
  String prompt = _("Identify list of persons for program:");
  Sequence *seq = 0;
  LLINES_PY_DATABASE *py_database = 0;
  Database *database;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|sO!", keywords, &prompt,
				     &llines_database_type, &py_database))
    return NULL;

  if (py_database)
    database = py_database->lld_database;
  else
    database = currentDatabase;

  seq = rptui_ask_for_indi_list (prompt, true, database);
  if (! seq)			/* user cancelled */
    Py_RETURN_NONE;

  PyObject *set = PySet_New (NULL);
  if (! set)
    return NULL;

  int len = lengthSequence (seq);
  if (len == 0)
    Py_RETURN_NONE;		/* XXX should this be an empty set instead? XXX */

  FORSEQUENCE (seq, element, count)
    LLINES_PY_RECORD *pyobj = PyObject_New (LLINES_PY_RECORD, &llines_record_type);
    if (! pyobj)
      {
	/* XXX need code to clean up XXX */
	return NULL;
      }
    pyobj->llr_type = LLINES_TYPE_INDI;
    pyobj->llr_record = element->root;
    pyobj->llr_database = database;
    PySet_Add (set, (PyObject *)pyobj);
  ENDSEQUENCE
  return (set);
}

/* llpy_menuchoose (choices, [prompt]) --> INT; Select from a collection of choices. */

static PyObject *llpy_menuchoose (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "choices", "prompt", NULL };
  PyObject *choices;
  char *prompt = _("Please choose from the following list.");
  char *c_string;
  int len;
  String *strings;
  int ndx;
  int answer;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "O|s", keywords, &choices, &prompt))
      return NULL;

  if (! PyList_Check (choices))
    {
      PyErr_SetString (PyExc_TypeError, _("menuchoose: CHOICES must be a list"));
      return NULL;
    }
  len = PyList_Size (choices);
  if (len <= 0)
    {
      PyErr_SetString (PyExc_IndexError, _("menuchoose: CHOICES must have a size > 0"));
      return NULL;
    }
  strings = (char **)stdalloc (len * sizeof (String));
  PyObject *tuple = PyTuple_New (1);
  for (ndx = 0; ndx < len; ndx++)
    {
      PyObject *item = PyList_GetItem(choices, ndx);
      if (! item)
	return NULL;
      PyTuple_SetItem (tuple, 0, item);
      PyArg_ParseTuple (tuple, "s", &c_string);
      strings[ndx] = c_string;
    }
  answer = rptui_chooseFromArray (prompt, len, strings);
  stdfree (strings);
  return Py_BuildValue ("i", answer);
}

static struct PyMethodDef Lifelines_User_Functions[] =
  {
   { "getindi",		(PyCFunction)llpy_getindi, METH_VARARGS | METH_KEYWORDS,
     "getindi([prompt],[database]) --> INDI; Identify person through user interface." },
   { "getindiset",		(PyCFunction)llpy_getindiset, METH_VARARGS | METH_KEYWORDS,
     "getindiset([prompt],[database]) --> SET; Identify set of persons through user interface." },
   { "getfam",		(PyCFunction)llpy_getfam, METH_NOARGS,
     "getfam([database]) --> FAM; Identify family through user interface." },
   { "getint",		(PyCFunction)llpy_getint, METH_VARARGS | METH_KEYWORDS,
     "getint([prompt]) --> INT; Get integer through user interface." },
   { "getstr",		(PyCFunction)llpy_getstr, METH_VARARGS | METH_KEYWORDS,
     "getstr([prompt]) --> STRING; Get string through user interface." },
   { "menuchoose",	(PyCFunction)llpy_menuchoose, METH_VARARGS | METH_KEYWORDS,
     "menuchoose(choices,[prompt]) --> INTEGER; Select from a collection of choices." },
   { NULL, 0, 0, NULL }		/* sentinel */
  };

void llpy_user_init (void)
{
  int status;

  status = PyModule_AddFunctions (Lifelines_Module, Lifelines_User_Functions);
  if (status != 0)
    fprintf (stderr, "llpy_user_init: attempt to add functions returned %d\n", status);
}

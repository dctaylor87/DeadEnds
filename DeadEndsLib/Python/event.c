/* event.c -- NODE event functions

   These are the bulk of the functions that are documented in the
   'NODE Event' section of the manual.  */


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
#include "date.h"		/* format_date */

#include "python-to-c.h"
#include "types.h"

#include "event.h"

/* forward references */

static PyObject *llpy_dayformat (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_monthformat (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_yearformat (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_eraformat (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_dateformat (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_complexformat (PyObject *self, PyObject *args, PyObject *kw);

/* might make this visible, for now it is private */
static struct py_dateformat
{
  int pyd_dayformat;		/* 0-2 */
  int pyd_monthformat;		/* 0-11 */
  int pyd_yearformat;		/* 0-2 */
  int pyd_eraformat;		/* 0-2,[123][12] */
  int pyd_dateformat;
  int pyd_complexformat;
} py_dateformat = { 0, 3, 0, 0, 0, 1};

PyObject *llpy_date (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *event = (LLINES_PY_NODE *) self;
  GNode *node = event->lnn_node;
  String str = eventToDate (node, false);

  if (! str)			/* no date recorded for event */
    Py_RETURN_NONE;

  return Py_BuildValue ("s", str);
}

PyObject *llpy_place (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *event = (LLINES_PY_NODE *) self;
  GNode *node = event->lnn_node;
  String str = eventToPlace (node, false);

  if (! str)			/* no place recorded for event */
    Py_RETURN_NONE;

  return Py_BuildValue ("s", str);
}

/* llpy_year -- this is non static because it must appear in the
   methods table found in nodes.c */
PyObject *llpy_year (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *event = (LLINES_PY_NODE *) self;
  CString str = eventToDate (event->lnn_node, true);

  if (! str)
    Py_RETURN_NONE;

  return Py_BuildValue ("s", str);
}

PyObject *llpy_long (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *event = (LLINES_PY_NODE *) self;
  GNode *node = event->lnn_node;
  String str = eventToString (node, false);

  if (! str)
    str = "";

  return Py_BuildValue ("s", str);
}

PyObject *llpy_short (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *event = (LLINES_PY_NODE *) self;
  GNode *node = event->lnn_node;
  String str = eventToString (node, true);
  if (! str)
    str = "";

  return Py_BuildValue ("s", str);
}

static PyObject *llpy_dayformat (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "format", NULL };
  int format = -1;
  int old_format = py_dateformat.pyd_dayformat;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|i", keywords, &format))
    return NULL;

  if ((format < -1) || (format > 2))
    {
      PyErr_SetString (PyExc_ValueError, "dayformat: format has a bad value, must be in the range [-1,2]");
      return NULL;
    }
  if (format != -1)
    py_dateformat.pyd_dayformat = format;

  return Py_BuildValue ("i", old_format);
}

static PyObject *llpy_monthformat (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "format", NULL };
  int format = -1;
  int old_format = py_dateformat.pyd_monthformat;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|i", keywords, &format))
    return NULL;

  if ((format < -1) || (format > 11))
    {
      PyErr_SetString (PyExc_ValueError, "monthformat: format has a bad value, must be in the range [-1,11]");
      return NULL;
    }
  if (format != -1)
    py_dateformat.pyd_monthformat = format;

  return Py_BuildValue ("i", old_format);
}

static PyObject *llpy_yearformat (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "format", NULL };
  int format = -1;
  int old_format = py_dateformat.pyd_yearformat;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|i", keywords, &format))
    return NULL;

  if ((format < -1) || (format > 2))
    {
      PyErr_SetString (PyExc_ValueError, "yearformat: format has a bad value, must be in the range [-1,2]");
      return NULL;
    }
  if (format != -1)
    py_dateformat.pyd_yearformat = format;

  return Py_BuildValue ("i", old_format);
}

static PyObject *llpy_eraformat (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "format", NULL };
  int format = -1;
  int old_format = py_dateformat.pyd_eraformat;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|i", keywords, &format))
    return NULL;

  switch (format)
    {
    case 0:
    case 1:
    case 2:
    case 11:
    case 12:
    case 21:
    case 22:
    case 31:
    case 32:
      py_dateformat.pyd_eraformat = format;
      /* FALL THROUGH */
    case -1:
      return Py_BuildValue ("i", old_format);

    default:
      PyErr_SetString (PyExc_ValueError, "eraformat: format has a bad value");
      return NULL;
    }
}

/* llpy_complexformat (format) -- format must be -1 or in the range
  [3,8].  The value -1 just returns the current value without changing
  it, values in the range [3,8] get set and return the previous value.
  The value says what to do with words such as 'about', 'between', and
  'estimated' that accompany the date.  Format values:

               upper case title case lower case
  abbreviation          3          4          7
  full word             5          6          8 */

static PyObject *llpy_complexformat (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "format", NULL };
  int format = -1;
  int old_format = py_dateformat.pyd_complexformat;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|i", keywords, &format))
    return NULL;

  if ((format >= 3) && (format <= 8))
    py_dateformat.pyd_complexformat = format;
  else if (format != -1)
    {
      PyErr_SetString (PyExc_ValueError, "complexformat: format has a bad value");
      return NULL;
    }
  return Py_BuildValue ("i", old_format);
}

#if !defined(DEADENDS)
static PyObject *llpy_complexpic (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "which", "format", NULL };
  int which = -1;
  char *format = NULL;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "i|z", keywords, &which, &format))
    return NULL;

  if ((which < 0) || (which >= ECMPLX_END))
    {
      PyErr_SetString (PyExc_ValueError, "complexpic: bad value for 'which'");
      return NULL;
    }
  if (format && (*format == '\0'))
    format = NULL;

  if (! set_cmplx_pic (which, format))
    {
      PyErr_SetString (PyExc_ValueError, "complexpic: bad argument");
      return NULL;
    }
  Py_RETURN_TRUE;
}
#endif

static PyObject *llpy_complexdate_str (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  String input_str;
  String output_str;
  static char *keywords[] = { "date", NULL };

  if (! PyArg_ParseTupleAndKeywords (args, kw, "s", keywords, &input_str))
    return NULL;

  output_str = do_format_date (input_str,
			       py_dateformat.pyd_dayformat,
			       py_dateformat.pyd_monthformat,
			       py_dateformat.pyd_yearformat,
			       py_dateformat.pyd_dateformat,
			       py_dateformat.pyd_eraformat,
			       py_dateformat.pyd_complexformat);

  return Py_BuildValue ("s", output_str);
}

PyObject *llpy_complexdate_node (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *py_node = (LLINES_PY_NODE *) self;
  String input_str;
  String output_str;

  input_str = eventToDate (py_node->lnn_node, false);
  output_str = do_format_date (input_str,
			       py_dateformat.pyd_dayformat,
			       py_dateformat.pyd_monthformat,
			       py_dateformat.pyd_yearformat,
			       py_dateformat.pyd_dateformat,
			       py_dateformat.pyd_eraformat,
			       py_dateformat.pyd_complexformat);

  return Py_BuildValue ("s", output_str);
}

/* llpy_stddate_str(String date) ==> String -- takes the given date,
   breaks it apart, formats it according to the previously specified
   formats, and returns the resulting string.  */

static PyObject *llpy_stddate_str  (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "date", NULL };
  char *input_date;
  char *output_date;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "s", keywords, &input_date))
    return NULL;

  output_date = do_format_date (input_date,
				py_dateformat.pyd_dayformat,
				py_dateformat.pyd_monthformat,
				py_dateformat.pyd_yearformat,
				py_dateformat.pyd_dateformat,
				py_dateformat.pyd_eraformat, false);

  return Py_BuildValue ("s", output_date);
}

/* llpy_stddate_node(NODE event) ==> String -- takes the given event,
   finds the date, breaks it apart, formats it according to the
   previously specified formats, and returns the resulting string.  */

PyObject *llpy_stddate_node  (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_NODE *py_node = (LLINES_PY_NODE *) self;
  char *input_date;
  char *output_date;

  input_date = eventToDate (py_node->lnn_node, false);

  output_date = do_format_date (input_date,
				py_dateformat.pyd_dayformat,
				py_dateformat.pyd_monthformat,
				py_dateformat.pyd_yearformat,
				py_dateformat.pyd_dateformat,
				py_dateformat.pyd_eraformat, false);

  return Py_BuildValue ("s", output_date);
}

static PyObject *llpy_dateformat (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "format", NULL };
  int format = -1;
  int old_format = py_dateformat.pyd_dateformat;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "|i", keywords, &format))
    return NULL;

  if ((format < -1) || (format > 14))
    {
      PyErr_SetString (PyExc_ValueError, "dateformat: format has a bad value, must be in the range [-1,2]");
      return NULL;
    }
  if (format != -1)
    py_dateformat.pyd_dateformat = format;

  return Py_BuildValue ("i", old_format);
}

static struct PyMethodDef Lifelines_Date_Functions[] =
  {
   { "dayformat",	(PyCFunction)llpy_dayformat, METH_VARARGS,
     "dayformat([format]) --> INT: set day format for stddate calls\n\n\
\t\t-1 ([default] do not change format, just return existing format)\n\
\t\t0 (leave space before single digit days)\n\
\t\t1 (use leading 0 before single digit days)\n\
\t\t2 (no space or leading 0 before single digit days)." },
   { "monthformat",	(PyCFunction)llpy_monthformat, METH_VARARGS,
     "monthformat([format]) --> INT: set month format for stddate calls.\n\n\
\t\t-1 ([default] do not change format, just return existing format)\n\
\t\t0\tnumber with space before single digit months\n\
\t\t1\tnumber with leading zero before single digit months\n\
\t\t2\tnumber with no space for zero before single digit months\n\
\t\t3\tupper case abbreviation (eg.g. JAN, FEB) (localized)\n\
\t\t4\tcapitalized abbreviation (e.g., Jan, Feb) (localized)\n\
\t\t5\tupper case full word (e.g., JANUARY, FEBRUARY) (localized)\n\
\t\t6\tcapitalized full word (e.g., January, February) (localized)\n\
\t\t7\tlower case abbreviations (e.g., jan, feb) (localized)\n\
\t\t8\tlower case full word (e.g., january, february) (localized)\n\
\t\t9\tupper case abbreviation in English per GEDCOM (e.g., JAN, FEB)\n\
\t\t10\tlower case roman letter (e.g., i, ii)\n\
\t\t11\tupper case roman letter (e.g., I, II)\n" },
   { "yearformat",	(PyCFunction)llpy_yearformat, METH_VARARGS,
     "yearformat(format) --> INT: set year format for stddate calls\n\n\
\t\t-1 ([default] do not change format, just return existing format)\n\
\t\t0 (use leading spaces before years with less than four digits)\n\
\t\t1 (use leading 0 before years with less than four digits\n\
\t\t2 (no space or leading 0 before years)." },
   { "eraformat",	(PyCFunction)llpy_eraformat, METH_VARARGS,
     "eraformat(format) --> INT: set era format for stddate calls\n\n\
\t\t-1 ([default] do not change format, just return existing format)\n\
\t\t0 (no AD/BC markers)\n\
\t\t1 (trailing B.C. if appropriate)\n\
\t\t2 (trailing A.D. or B.C.)\n\
\t\t11 (trailing BC if appropriate)\n\
\t\t12 (trailing AD or BC)\n\
\t\t21 (trailing B.C.E. if appropriate)\n\
\t\t22 (trailing C.E. or B.C.E.)\n\
\t\t31 (trailing BC if appropriate)\n\
\t\t32 (trailing CE or BCE)." },
   { "dateformat",	(PyCFunction)llpy_dateformat, METH_VARARGS,
     "dateformat(format) --> INT: set date format for stddate calls.\n\n\
\t\t-1 ([default] do not change format, just return existing format\n\
\t\t0  (da mo yr)\n\
\t\t1  (mo da, yr)\n\
\t\t2  (mo/da/yr)\n\
\t\t3  (da/mo/yr)\n\
\t\t4  (mo-da-yr)\n\
\t\t5  (da-mo-yr)\n\
\t\t6  (modayr)\n\
\t\t7  (damoyr)\n\
\t\t8  (yr mo da)\n\
\t\t9  (yr/mo/da)\n\
\t\t10 (yr-mo-da)\n\
\t\t11 (yrmoda)\n\
\t\t12 (yr, year only omitting all else)\n\
\t\t13 (da/mo yr)\n\
\t\t14 (As in GEDCOM" },
   { "stddate",		(PyCFunction)llpy_stddate_str,
     METH_VARARGS | METH_KEYWORDS,
     "stddate(date) --> STRING: formatted date string." },

   { "complexformat",	(PyCFunction)llpy_complexformat,
     METH_VARARGS | METH_KEYWORDS,
     "complexformat([INT format]) --> INT:\n\n\
Format must be either -1 or be in the range [3,8].\n\
The value -1 just returns the current value without changing it.  Values in\n\
the range [3,8] get set and return the previous value.  The value says what\n\
to do with the words such as 'about', 'between', and 'estimated' that\n\
accompany the date.  Format values:\n\
\t3 abbreviated, upper case\n\
\t4 abbreviated, title case\n\
\t7 abbreviated, lower case\n\
\t5 full words,  upper case\n\
\t6 full words,  title case\n\
\t8 full words,  lower case" },
   { "complexdate",	(PyCFunction)llpy_complexdate_str,
     METH_VARARGS | METH_KEYWORDS,
     "complexdate(date) --> STRING: formats and returns date\n\n\
using complex date formats previously specified." },

#if !defined(DEADENDS)
   { "complexpic",	(PyCFunction)llpy_complexpic,
     METH_VARARGS | METH_KEYWORDS,
     "complexpic(which,[format] --> BOOLEAN\n\n\
'which' is one of the constants: DATE_COMPLEX_ABT, DATE_COMPLEX_EST,\n\
DATE_COMPLEX_CAL, DATE_COMPLEX_BEF, DATE_COMPLEX_AFT, DATE_COMPLEX_BET_AND,\n\
DATE_COMPLEX_FROM, DATE_COMPLEX_TO, and DATE_COMPLEX_FROM_TO.\n\
And 'format' is a STRING or None." },
#endif
   { NULL, 0, 0, NULL }		/* sentinel */
  };

void llpy_event_init (void)
{
  int status;

  status = PyModule_AddFunctions (Lifelines_Module, Lifelines_Date_Functions);
  if (status != 0)
    fprintf (stderr, "llpy_event_init: attempt to add functions returned %d\n", status);

#if !defined(DEADENDS)
  status = PyModule_AddIntConstant (Lifelines_Module, "DATE_COMPLEX_ABT", ECMPLX_ABT);
  if (status != 0)
    fprintf (stderr,
	     "llpy_event_init: attempt to add int constant DATE_COMPLEX_ABT returned %d\n",
	     status);

  status = PyModule_AddIntConstant (Lifelines_Module, "DATE_COMPLEX_EST", ECMPLX_EST);
  if (status != 0)
    fprintf (stderr,
	     "llpy_event_init: attempt to add int constant DATE_COMPLEX_EST returned %d\n",
	     status);

  status = PyModule_AddIntConstant (Lifelines_Module, "DATE_COMPLEX_CAL", ECMPLX_CAL);
  if (status != 0)
    fprintf (stderr,
	     "llpy_event_init: attempt to add int constant DATE_COMPLEX_CAL returned %d\n",
	     status);

  status = PyModule_AddIntConstant (Lifelines_Module, "DATE_COMPLEX_BEF", ECMPLX_BEF);
  if (status != 0)
    fprintf (stderr,
	     "llpy_event_init: attempt to add int constant DATE_COMPLEX_BEF returned %d\n",
	     status);

  status = PyModule_AddIntConstant (Lifelines_Module, "DATE_COMPLEX_AFT", ECMPLX_AFT);
  if (status != 0)
    fprintf (stderr,
	     "llpy_event_init: attempt to add int constant DATE_COMPLEX_AFT returned %d\n",
	     status);

  status = PyModule_AddIntConstant (Lifelines_Module, "DATE_COMPLEX_BET_AND", ECMPLX_BET_AND);
  if (status != 0)
    fprintf (stderr,
	     "llpy_event_init: attempt to add int constant DATE_COMPLEX_BET_AND returned %d\n",
	     status);

  status = PyModule_AddIntConstant (Lifelines_Module, "DATE_COMPLEX_FROM", ECMPLX_FROM);
  if (status != 0)
    fprintf (stderr,
	     "llpy_event_init: attempt to add int constant DATE_COMPLEX_FROM returned %d\n",
	     status);

  status = PyModule_AddIntConstant (Lifelines_Module, "DATE_COMPLEX_TO", ECMPLX_TO);
  if (status != 0)
    fprintf (stderr,
	     "llpy_event_init: attempt to add int constant DATE_COMPLEX_TO returned %d\n",
	     status);

  status = PyModule_AddIntConstant (Lifelines_Module, "DATE_COMPLEX_FROM_TO", ECMPLX_FROM_TO);
  if (status != 0)
    fprintf (stderr,
	     "llpy_event_init: attempt to add int constant DATE_COMPLEX_FROM_TO returned %d\n",
	     status);
#endif
}

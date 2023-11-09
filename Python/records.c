/* records.c -- RECORD relatted funcions which are not specific to a
   particular RECORD type. */

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

#include "gedcom.h"		/* RECORD */
#include "indiseq.h"		/* INDISEQ */
#include "liflines.h"
#endif
#include "python-to-c.h"
#include "types.h"

/* forware references */

static PyObject *llpy_key_to_record (PyObject *self, PyObject *args, PyObject *kw);
static PyObject *llpy_keynum_to_record (PyObject *self, PyObject *args, PyObject *kw);

/* start of code */

#if defined(DEADENDS)
static PyObject *llpy_key_to_record (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "key", "type", NULL };
  const char *key = 0;
  const char *type = 0;
  int int_type = 0;
  RECORD record;
  LLINES_PY_RECORD *py_record;
  int added_at = 0;
  int ndx;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "s|z", keywords, &key, &type))
    return NULL;

  char key_buffer[strlen(key) + 3];

  /* convenience -- add '@'s if needed; convert to uppercase */
  if (key[0] != '@')
    {
      key_buffer[0] = '@';
      added_at = 1;
    }
  for (ndx = 0; key[ndx]; ndx++)
    key_buffer[ndx + added_at] = toupper(key[ndx]);
  if (added_at)
    key_buffer[ndx++ + added_at] = '@';

  key_buffer[ndx + added_at] = 0;

  if (type && (type[0] != 0))
    {
      /* if type was specified and is not the empty string, we search
	 only those types of records */
      switch (type[0])
	{
	case 'F':
	  if (eqstr (type, "FAM"))
	    {
	      int_type = 'F';
	      break;
	    }
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;

	case 'I':
	  if (eqstr (type, "INDI"))
	    {
	      int_type = 'I';
	      break;
	    }
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;

	case 'S':
	  if (eqstr (type, "SOUR"))
	    {
	      int_type = 'S';
	      break;
	    }
	  else if (eqstr (type, "SUBM") || (eqstr (type, "SNOTE")))
	    {
	      int_type = 'X';
	      break;
	    }
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;

	case 'E':
	  if (eqstr (type, "EVEN"))
	    {
	      int_type = 'E';
	      break;
	    }
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;

	case 'R':
	  if (eqstr (type, "REPO"))
	    {
	      int_type = 'X';
	      break;
	    }
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;

	case 'O':
	  if (eqstr (type, "OBJE"))
	    {
	      int_type = 'X';
	      break;
	    }
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;

	default:
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;
	}

      /* if the key has a prefix, verify that it matches the computed type */
      if (! isdigit (key[0]) && (key[0] != int_type))
	{
	  /* key has prefix, but it does not match type's prefix */
	  PyErr_SetString (PyExc_ValueError, "key_to_record: key's prefix incompatible with type");
	  return NULL;
	}
    }

  switch (int_type)
    {
    case 'I':
      record = qkey_to_irecord (key_buffer);
      break;
    case 'F':
      record = qkey_to_frecord (key_buffer);
      break;
    case 'S':
      record = qkey_to_srecord (key_buffer);
      break;
    case 'E':
      record = qkey_to_erecord (key_buffer);
      break;
    case 'X':
      record = qkey_to_orecord (key_buffer);
      break;
    default:
      /* caller did not specify a type, try them all until we find it.
	 If all fail, we will return None */
      int_type = 'I';
      record = qkey_to_irecord (key_buffer);
      if (! record)
	{
	  int_type = 'F';
	  record = qkey_to_frecord (key_buffer);
	}
      if (! record)
	{
	  int_type = 'S';
	  record = qkey_to_srecord (key_buffer);
	}
      if (! record)
	{
	  int_type = 'E';
	  record = qkey_to_erecord (key_buffer);
	}
      if (! record)
	{
	  int_type = 'X';
	  record = qkey_to_orecord (key_buffer);
	}
    }

  if (! record)
    Py_RETURN_NONE;		/* that keynum has no record */

  switch (int_type)
    {
    case 'I':
      py_record = (LLINES_PY_RECORD *) PyObject_New (LLINES_PY_RECORD,
						     &llines_individual_type);
      break;
    case 'F':
      py_record = (LLINES_PY_RECORD *) PyObject_New (LLINES_PY_RECORD,
						     &llines_family_type);
      break;
    case 'S':
      py_record = (LLINES_PY_RECORD *) PyObject_New (LLINES_PY_RECORD,
						     &llines_source_type);
      break;
    case 'E':
      py_record = (LLINES_PY_RECORD *) PyObject_New (LLINES_PY_RECORD,
						     &llines_event_type);
      break;
    case 'X':
      py_record = (LLINES_PY_RECORD *) PyObject_New (LLINES_PY_RECORD,
						     &llines_other_type);
      break;
    }
  if (! py_record)
    return NULL;

  py_record->llr_record = record;
  py_record->llr_type = int_type;
  return ((PyObject *) py_record);
}
#else
static PyObject *llpy_key_to_record (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "key", "type", NULL };
  char *key = 0;
  char *type = 0;
  char key_buffer[22]; /* prefix (1) + unsigned long (<=20) + nul (1) */
  int int_type = 0;
  RECORD record;
  LLINES_PY_RECORD *py_record;
  int use_keybuf = 0;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "s|z", keywords, &key, &type))
    return NULL;

  /* convenience -- if a node has a xref, NODE.value() returns it with
     @'s around it -- this saves the user from having to strip them
     off */
  if (key[0] == '@')
    key = rmvat (key);

  if (type)
    {
      switch (type[0])
	{
	case 'F':
	  if (eqstr (type, "FAM"))
	    {
	      int_type = 'F';
	      break;
	    }
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;

	case 'I':
	  if (eqstr (type, "INDI"))
	    {
	      int_type = 'I';
	      break;
	    }
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;

	case 'S':
	  if (eqstr (type, "SOUR"))
	    {
	      int_type = 'S';
	      break;
	    }
	  else if (eqstr (type, "SUBM") || (eqstr (type, "SNOTE")))
	    {
	      int_type = 'O';
	      break;
	    }
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;

	case 'E':
	  if (eqstr (type, "EVEN"))
	    {
	      int_type = 'E';
	      break;
	    }
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;

	case 'R':
	  if (eqstr (type, "REPO"))
	    {
	      int_type = 'X';
	      break;
	    }
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;

	case 'O':
	  if (eqstr (type, "OBJE"))
	    {
	      int_type = 'X';
	      break;
	    }
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;

	default:
	  PyErr_SetString (PyExc_ValueError, "key_to_record: TYPE has a bad value");
	  return NULL;
	}

      /* if the key has a prefix, verify that it matches the computed type */
      if (! isdigit (key[0]) && (key[0] != int_type))
	{
	  /* key has prefix, but it does not match type's prefix */
	  PyErr_SetString (PyExc_ValueError, "key_to_record: key's prefix incompatible with type");
	  return NULL;
	}
    }
  else
    {
      /* type was not specified, verify that key starts with an upper case letter */
      if (! isupper(key[0]))
	{
	  PyErr_SetString (PyExc_TypeError, "key_to_record: key has no prefix and type not specified");
	  return NULL;
	}
      int_type = key[0];
    }


  if (isdigit (key[0]))
    {
      if (strlen (key) > (sizeof (key_buffer) - 2))
	{
	  PyErr_SetString (PyExc_ValueError, "key_to_record: key too long");
	  return NULL;
	}
      snprintf (key_buffer, sizeof (key_buffer), "%c%s", int_type, key);
      use_keybuf = 1;
    }
  else
    {
      int_type = key[0];
      use_keybuf = 0;
    }

  switch (int_type)
    {
    case 'I':
      record = qkey_to_irecord (use_keybuf ? key_buffer : key);
      break;
    case 'F':
      record = qkey_to_frecord (use_keybuf ? key_buffer : key);
      break;
    case 'S':
      record = qkey_to_srecord (use_keybuf ? key_buffer : key);
      break;
    case 'E':
      record = qkey_to_erecord (use_keybuf ? key_buffer : key);
      break;
    case 'X':
      record = qkey_to_orecord (use_keybuf ? key_buffer : key);
      break;
    default:
      PyErr_SetString (PyExc_ValueError, "key_to_record: bad key value");
      return NULL;
    }

  if (! record)
    Py_RETURN_NONE;		/* that keynum has no record */

  switch (int_type)
    {
    case 'I':
      py_record = (LLINES_PY_RECORD *) PyObject_New (LLINES_PY_RECORD,
						     &llines_individual_type);
      break;
    case 'F':
      py_record = (LLINES_PY_RECORD *) PyObject_New (LLINES_PY_RECORD,
						     &llines_family_type);
      break;
    case 'S':
      py_record = (LLINES_PY_RECORD *) PyObject_New (LLINES_PY_RECORD,
						     &llines_source_type);
      break;
    case 'E':
      py_record = (LLINES_PY_RECORD *) PyObject_New (LLINES_PY_RECORD,
						     &llines_event_type);
      break;
    case 'X':
      py_record = (LLINES_PY_RECORD *) PyObject_New (LLINES_PY_RECORD,
						     &llines_other_type);
      break;
    }
  if (! py_record)
    return NULL;

  py_record->llr_record = record;
  py_record->llr_type = int_type;
  return ((PyObject *) py_record);
}
#endif

static PyObject *llpy_keynum_to_record (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "keynum", "type", NULL };
  unsigned long keynum = 0;
  char *type = 0;
  char key_buffer[22]; /* prefix (1) + unsigned long (<=20) + nul (1) */
  int int_type = 0;
  RECORD record;
  LLINES_PY_RECORD *py_record;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "ks", keywords, &keynum, &type))
    return NULL;

  switch (type[0])
    {
    case 'F':
      if (eqstr (type, "FAM"))
	{
	  int_type = 'F';
	  break;
	}
      PyErr_SetString (PyExc_ValueError, "keynum_to_record: TYPE has a bad value");
      return NULL;

    case 'I':
      if (eqstr (type, "INDI"))
	{
	  int_type = 'I';
	  break;
	}
      PyErr_SetString (PyExc_ValueError, "keynum_to_record: TYPE has a bad value");
      return NULL;

    case 'S':
      if (eqstr (type, "SOUR"))
	{
	  int_type = 'S';
	  break;
	}
      else if (eqstr (type, "SUBM") || (eqstr (type, "SNOTE")))
	{
	  int_type = 'O';
	  break;
	}
      PyErr_SetString (PyExc_ValueError, "keynum_to_record: TYPE has a bad value");
      return NULL;

    case 'E':
      if (eqstr (type, "EVEN"))
	{
	  int_type = 'E';
	  break;
	}
      PyErr_SetString (PyExc_ValueError, "keynum_to_record: TYPE has a bad value");
      return NULL;

    case 'R':
      if (eqstr (type, "REPO"))
	{
	  int_type = 'X';
	  break;
	}
      PyErr_SetString (PyExc_ValueError, "keynum_to_record: TYPE has a bad value");
      return NULL;

    case 'O':
      if (eqstr (type, "OBJE"))
	{
	  int_type = 'X';
	  break;
	}
      PyErr_SetString (PyExc_ValueError, "keynum_to_record: TYPE has a bad value");
      return NULL;

    default:
      PyErr_SetString (PyExc_ValueError, "keynum_to_record: TYPE has a bad value");
      return NULL;
    }
  snprintf (key_buffer, sizeof (key_buffer), "%c%lu", int_type, keynum);

  switch (int_type)
    {
    case 'I':
      record = qkey_to_irecord (key_buffer);
      break;
    case 'F':
      record = qkey_to_frecord (key_buffer);
      break;
    case 'S':
      record = qkey_to_srecord (key_buffer);
      break;
    case 'E':
      record = qkey_to_erecord (key_buffer);
      break;
    case 'X':
      record = qkey_to_orecord (key_buffer);
      break;
    default:
      PyErr_SetString (PyExc_TypeError, "keynum_to_record: bad type");
      return NULL;
    }
  if (! record)
    Py_RETURN_NONE;		/* that keynum has no record */

  switch (int_type)
    {
    case 'I':
    case 'F':
    case 'S':
    case 'E':
    case 'X':
      py_record = (LLINES_PY_RECORD *) PyObject_New (LLINES_PY_RECORD,
						     &llines_individual_type);
      break;

    default:
      py_record = 0;
      break;
    }
  if (! py_record)
    return NULL;

  py_record->llr_record = record;
  py_record->llr_type = int_type;
  return ((PyObject *) py_record);
}


static struct PyMethodDef Lifelines_Records_Functions[] =
  {
   { "key_to_record",	(PyCFunction)llpy_key_to_record, METH_VARARGS | METH_KEYWORDS,
     "key_to_record(key,[type]) --> RECORD.  Returns the RECORD having that KEY.\n\
If KEY contains the record type, then TYPE is optional, otherwise required.\n\
TYPE when supplied must be the level zero RECORD tag -- INDI, FAM, SOUR, EVEN,\n\
REPO, SUBM, SNOTE, or OBJE.  If the record is not found, None is returned." },
   { "keynum_to_record",	(PyCFunction)llpy_keynum_to_record, METH_VARARGS | METH_KEYWORDS,
     "keynum_to_record(key,type) --> RECORD or None.\n\
Returns the RECORD having the specified KEYNUM and TYPE.\n\
TYPE must be the level zero RECORD tag -- INDI, FAM, SOUR, EVEN,\n\
REPO, SUBM, SNOTE, or OBJE.  If the record is not found, None is returned." },

   { NULL, 0, 0, NULL }		/* sentinel */
  };

void llpy_records_init (void)
{
  int status;

  status = PyModule_AddFunctions (Lifelines_Module, Lifelines_Records_Functions);
  if (status != 0)
    fprintf (stderr, "llpy_records_init: attempt to add functions returned %d\n", status);
}

/* run.c -- functions for running the Python interpreter -- if it is
   compiled in */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>		/* ATTRIBUTE_UNUSED */
#include <stdio.h>
#include <stdint.h>

#include "porting.h"		/* LifeLines --> DeadEnds */
#include "standard.h"		/* String */
#include "denls.h"
#include "path.h"		/* fopenPath */
#include "refnindex.h"
#include "gnode.h"
#include "recordindex.h"
#include "locales.h"
#include "lloptions.h"

#include "python-to-c.h"
#include "llpy-externs.h"

/* we have three things to handle:

. calling Python interactively if the user specified -P

. 'registering' each script specified on the command line

. calling the scripts */

/* forward references */

static int execute_script (CString script);

/* start of code */

/* array of scripts seen on the command line, in the order seen */
static char **script_array;

/* how many scripts we've seen */
static int nscripts = 0;

/* so that one time initialization is only run once */
static int initialized = 0;

/* llpy_register_script -- we record or register the sciripts seen on
   the command line.  We don't try to be fancy and optimize the calls
   to allocate space for the script names as typically the user will
   only specify one script or possibly two.

   TODO: figure out how to do script specific arguments and options. */

int
llpy_register_script (char *script)
{
  char **new_array;

  if (nscripts)
    new_array = (char **)stdrealloc ((void *)script_array, (nscripts + 1) * sizeof (char *));
  else
    new_array = (char **)stdalloc (sizeof (char *));

  if (! new_array)
    return (-1);		/* malloc / realloc failed! */

  new_array[nscripts] = script;
  script_array = new_array;
  nscripts++;
  return (0);
}

/* llpy_execute_scripts -- execute the scripts in the order specified
   on the command line.

   continue_on_failure -- if true and a script fails, we continue to
   the next script, otherwise we return with the failure status.

   TODO: make continue on failure script specific.

   FUTURE TODO: figure out how to pass arguments and options to the
   script -- something better than reading stdin for arguments. */

int
llpy_execute_scripts (int continue_on_failure)
{
  int saved_status = 0;

  if (! initialized)
    {
      llpy_init ();
      Py_Initialize ();
      initialized = 1;
    }

  for (int cur_script = 0; cur_script < nscripts; cur_script++)
    {
      int status = execute_script (script_array[cur_script]);
      if (status < 0)
	{
	  if (! continue_on_failure)
	    return (status);
	  else
	    saved_status = status;
	}
    }
  return (saved_status);	/* most recent failure, if any, otherwise zero */
}

static int
execute_script (CString report_name)
{
  FILE *report_fp;
  /* XXX might want to open in binary mode -- LLREADBINARY XXX */
#if defined(DEADENDS)
  String mode = "r";
#else
  String mode = DEREADTEXT;	/* "r" on POSIX, on M$ systems this is "rt" */
  String ext = ".py";
  int utf8 = uu8;
  String report_pathname;
#endif
  char *path;
  int status = 0;
  PyGILState_STATE py_gil;	/* python's GIL */
#if 1
  PyObject *main_module;
  PyObject *dict;
#endif

  path = getdeoptstr ("DEPROGRAMS", ".");
#if defined(DEADENDS)
  report_fp = fopenPath (report_name, mode, path);
#else
  report_fp = fopenpath (report_name, mode, path, ext, utf8, &report_pathname);
#endif

  if (! report_fp)
    {
      /* give the bad news */
      fprintf (stderr, "unable to open %s\n", report_name);
      return (-1);
    }
  py_gil = PyGILState_Ensure();
#if 0
  status = PyRun_SimpleFile (report_fp, report_pathname);
#else
  main_module = PyImport_AddModule ("__main__");
  if (! main_module)
    {
      PyErr_Print ();
      fclose (report_fp);
      return (-1);
    }
  else
    {
      Py_INCREF (main_module);
      dict = PyModule_GetDict (main_module);
      if (PyRun_File (report_fp, report_name, Py_file_input, dict, dict) == NULL)
	{
	  /* failure, if it was exit with zero status, return success anyway */
	  Py_CLEAR (main_module);
	  PyObject *exception_type = PyErr_Occurred ();
	  if (! PyErr_GivenExceptionMatches (exception_type, PyExc_SystemExit))
	    {
	      /* not an exit */
	      PyErr_Print ();
	      status = -1;
	    }
	  else
	    {
	      /* it was an 'exit' -- was it a call to exit?  Or a signal? */
	      PyObject *type;
	      PyObject *value;
	      PyObject *traceback;
	      long exit_code;

	      PyErr_Fetch (&type, &value, &traceback);
	      exit_code = PyLong_AsLong (value);
	      if (exit_code == 0)
		status = 0;
	      else
		{
		  /* was it a call to exit? */
		  if (WIFEXITED((int)exit_code))
		    status = WEXITSTATUS ((int)exit_code);
		  else
		    {
		      /* XXX insert code to print a traceback XXX */
		      status = -1;
		    }
		}
	      Py_XDECREF (type);
	      Py_XDECREF (value);
	      Py_XDECREF (traceback);
	    }
	}
      Py_XDECREF (main_module);
    }
#endif
  PyGILState_Release(py_gil);

  fclose (report_fp);
  return (status);
}

int
llpy_python_interactive (void)
{
  int status;

  if (! initialized)
    {
      llpy_init ();
      Py_Initialize ();
      initialized = 1;
    }
  status = PyRun_InteractiveLoop (stdin, "<stdin>");
  return (status);
}

/* llpy_python_terminate -- called during LifeLines shutdown.  If
   Python has been used, free any Python objects and hence decrement
   the corresponding NODE and RECORD reference counts */

void
llpy_python_terminate (void)
{
  if (initialized)
    {
      (void) Py_FinalizeEx ();
      initialized = 0;
    }
}

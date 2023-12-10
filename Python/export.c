/* export.c -- support for exporting a database to a GEDCOM file.

   Eventually this will likely be extended to do both export and
   import.  At that time it might be renamed. */

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#if defined(HAVE_CONFIG_H)
#include "config.h"
#endif

#include <ansidecl.h>
#include <stdio.h>
#include <assert.h>

#include "porting.h"

#include "standard.h"
#include "llnls.h"
#include "database.h"
#include "gnode.h"

#include "python-to-c.h"
#include "types.h"

/* forward references */

static PyObject *llpy_export (PyObject *self, PyObject *args, PyObject *kw);

static int write_header (FILE *file,
			 const char *version,
			 const char *submitter);
static int write_body (FILE *file, Database *database);
static int write_all_records (FILE *file, RecordIndex *index);
static int write_node_tree (FILE *file, GNode *root);
static int write_out_node (GNode *node, int level, void *extra);
static int write_trailer (FILE *file);

static bool valid_submitter (CString submitter);
static bool valid_gedcom_version (CString gedcom_version);

/* start of code */

static PyObject *
llpy_export (PyObject *self ATTRIBUTE_UNUSED, PyObject *args, PyObject *kw)
{
  static char *keywords[] = { "file", "version", "submitter", "database", NULL };
  char *filename = 0;
  char *gedcom_version = 0;
  char *submitter = 0;
  LLINES_PY_DATABASE *py_db = 0;
  Database *database = 0;

  if (! PyArg_ParseTupleAndKeywords (args, kw, "s|zzO!", keywords,
				     &filename, &gedcom_version, &submitter,
				     &llines_database_type, &py_db))
    return NULL;

  /* if database specified use it, otherwise use the current default database */
  if (! py_db)
    database = theDatabase;
  else
    database = py_db->lld_database;

  return _llpy_export (database, filename, gedcom_version, submitter);
}

/* this is the implementation of the commonality of llpy_export /
   llpy_export_db -- basically everything aftger the argument
   parsing. */

PyObject *
_llpy_export (Database *database, CString filename,
	       CString gedcom_version, CString submitter)
{
  FILE *file;

  file = fopen (filename, "w");
  if (! file)
    {
      PyErr_SetString (PyExc_OSError, "export: cannot open file for writing");
      return NULL;
    }

  /* treat an empty submitter string the same as not supplied at all */
  if (submitter && (submitter[0] != '\0'))
    {
      if (! valid_submitter (submitter))
	{
	  PyErr_SetString (PyExc_ValueError, "export: invalid submitter value");
	  return NULL;
	}
      /* normalize submitter -- user convenience -- this allows user
	 to type a lower case value without @'s */
      int len = strlen (submitter);
      int added_at = 0;
      int ndx;
      char copy[len + 3];	/* len + 2 '@'s + NUL */

      if (submitter[0] != '@')
	{
	  copy[0] = '@';
	  added_at = 1;
	}
      for (ndx = 0; submitter[ndx]; ndx++)
	copy[ndx + added_at] = toupper(submitter[ndx]);
      if (added_at)
	copy[ndx++ + added_at] = '@';
      copy[ndx + added_at] = 0;
      submitter = copy;
      /* XXX insert code to verify existence of submitter in database XXX */
    }
  else
    submitter = 0;
    

  if (! gedcom_version || (gedcom_version[0] == '\0'))
    gedcom_version = "5.5";
  else if (! valid_gedcom_version (gedcom_version))
    {
      PyErr_SetString (PyExc_ValueError, "export: invalid GEDCOM version value");
      return NULL;
    }

  if ((write_header (file, gedcom_version, submitter) < 0) ||
      (write_body (file, database) < 0) ||
      (write_trailer(file) < 0) ||
      (fflush (file) < 0))
    {
      PyErr_SetString (PyExc_OSError, "export: error writing to file");
      return NULL;
    }
  fclose (file);

  Py_RETURN_NONE;
}
/* write_header -- write a simple, basic header
   This might get enhanced in the future.  */

static int
write_header (FILE *file,
	      const char *gedcom_version,
	      const char *submitter)
{
  const char *mon_str[12] =
    {
      "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
      "JUL", "AUG", "SEP", "OCT", "NOV", "DEC"
    };
  struct timespec ts;
  struct tm *tmp;

  if (! gedcom_version)
    version = "7.0.13";

  if (clock_gettime (CLOCK_REALTIME, &ts) < 0)
    return (-1);

  tmp = localtime (&ts.tv_sec);
  if (! tmp)
    return (-1);

  if (fputs ("0 HEAD\n1 GEDC\n", file) < 0)
    return (-1);
  if (fprintf (file, "2 VERS %s\n1 SOUR %s\n2 VERS %s\n1 DEST ANY\n",
	       gedcom_version, PACKAGE_NAME, version) < 0)
    return (-1);
  if (fprintf (file, "1 DATE %d %s %d\n2 TIME %d:%d\n",
	       tmp->tm_mday, mon_str[tmp->tm_mon], tmp->tm_year + 1900,
	       tmp->tm_hour, tmp->tm_min) < 0)
    return (-1);

  if (submitter)
    {
      if (fprintf (file, "1 SUBM %s\n", submitter) < 0)
	return (-1);
    }
  return (0);
}

static int
write_body (FILE *file, Database *database)
{
  /* Individuals */
  write_all_records (file, database->personIndex);
  /* Families */
  write_all_records (file, database->familyIndex);
  /* Sources */
  write_all_records (file, database->sourceIndex);
  /* Events */
  write_all_records (file, database->eventIndex);
  /* Others */
  write_all_records (file, database->otherIndex);
  return (-1);
}

static int
write_all_records (FILE *file, RecordIndex *index)
{
  RecordIndexEl *record;
  int bucket_ndx = -1;
  int element_ndx = -1;

  for (record = (RecordIndexEl *) firstInHashTable (index, &bucket_ndx, &element_ndx);
       record;
       record = (RecordIndexEl *) nextInHashTable (index, &bucket_ndx, &element_ndx))
    {
      GNode *root = record->root;
      if (write_node_tree (file, root) < 0)
	return (-1);
    }
  return (0);
}

static int
write_node_tree (FILE *file, GNode *root)
{
  return (_py_traverse_nodes (root, 0, write_out_node, (void *)file));
}

static int
write_out_node (GNode *node, int level, void *extra)
{
  FILE *file = (FILE *)extra;
  CString xref = node->key;
  CString tag = node->tag;
  CString value = node->value;
  int status;

  if (xref && (xref[0] != '\0'))
    status = fprintf (file, "%d %s ", level, xref);
  else
    status = fprintf (file, "%d ", level);

  if (status < 0)
    return (-1);

  if (value && (value[0] != '\0'))
    status = fprintf (file, "%s %s\n", tag, value);
  else
    status = fprintf (file, "%s\n", tag);

  if (status < 0)
    return (-1);

  return (1);			/* continue normally */
}

/* _py_traverse_nodes -- this is inspired by traverseNodes in
   Gedcom/gnode.c, with three very imporatant differences:

   . we return success or failure, not void

   . There are three return values (not two) from the function called
   on each node:

   --  1 success, continue normally
   --  0 success, done (early)
   -- -1 error, return failure

   . The function called takes three arguments -- the node, the level,
   and an opaque pointer -- instead of the two taken by traverseNodes.
   The opaque pointer allows us to avoid communication via external
   variables.  */

/* NOTE: while there are three return values from 'func' recognized by
   _py_traverse_nodes, depending upon 'func', there might only be two
   meaningful return values for a particular call or use case --

   e.g. if we are searching for a node, retval=1 might well mean keep
   searching and retval=0 mean 'found it, recorded it in 'extra'.

   Alternatively, if we are writing out nodes (e.g., as part of export),
   then retval=-1 might mean 'we encountered an error writing out the
   node' and 'retval=1' mean 'do the next node', */

int
_py_traverse_nodes (GNode *node, int level,
		    int (*func)(GNode *, int, void *),
		    void *extra)
{
  assert (node);
  assert (func);

  for (; node; node = node->sibling)
    {
      int retval = (*func)(node, level, extra);

      if (retval <= 0)
	return (retval);

      if (node->child)
	{
	  retval = _py_traverse_nodes (node->child, level+1, func, extra);
	  if (retval <= 0)
	    return (retval);
	}
    }
  return (1);
}

static int
write_trailer (FILE *file)
{
  return fputs ("0 TRLR\n", file);
}

/* valid_submitter -- for a submitter to be valid, it must be a key
   and it must point to a SUBM record */
static bool
valid_submitter (CString submitter)
{
  RECORD record;
  int type = 'X';

  record = __llpy_key_to_record (submitter, &type);
  if (! record)
    return false;

  if (nestr (record->root->tag, "SUBM"))
    return false;

  return true;
}

/* valid_gedcom_version -- simplistic validation */
static bool
valid_gedcom_version (CString gedcom_version)
{
  if (! gedcom_version)
    return true;		/* NULL pointer --> use default version */
  for (; *gedcom_version; gedcom_version++)
    if ((*gedcom_version != '.') && ! isdigit (*gedcom_version))
      return false;

  return true;			/* it consists solely of '.'s and digits */
}

static PyMethodDef Lifelines_Database_Functions2[] =
  {
    { "export",		(PyCFunction)llpy_export, METH_VARARGS | METH_KEYWORDS,
      "export(file, [version], [submitter]) --> None.  Exports the database to 'file'\n\n\
If 'version' and/or 'submitter' is specified, they are recorded in the header.\n\
Version is the GEDCOM version.  If omitted, a default version is used.\n\
Submitter is the submitter of the GEDCOM file.  If specified, it must point\n\
to a SUBM record.  If either 'version' or 'submitter' is invalid or 'file'\n\
cannot be opened for writing, an exception occurs." },
    { NULL, 0, 0, NULL }	/* sentinel */
  };

void llpy_export_init (void)
{
  int status;

  status = PyModule_AddFunctions (Lifelines_Module,
				  Lifelines_Database_Functions2);
  if (status != 0)
    fprintf (stderr, "llpy_export_init: attempt to add functions returned %d\n",
	     status);
}

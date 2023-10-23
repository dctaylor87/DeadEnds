/* family.c -- family functions.

   These are the bulk of the functions that are documented in the
   'Family functions' section of the manual.  */

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
#include "database.h"
#include "recordindex.h"	/* RecordIndexEl */
#include "nodeutils.h"		/* equal_tree */
#include "py-messages.h"
#else
#include "standard.h"		/* STRING */
#include "llstdlib.h"		/* CALLBACK_FNC */
#include "lloptions.h"

#include "gedcom.h"		/* RECORD */
#include "indiseq.h"		/* INDISEQ */
#include "liflines.h"		/* choose_from_indiseq */
#include "messages.h"
#include "../gedlib/leaksi.h"
#endif
#include "python-to-c.h"
#include "types.h"
#include "py-set.h"
#include "family.h"

/* forward references */

static PyObject *llpy_marriage (PyObject *self, PyObject *args);
static PyObject *llpy_husband (PyObject *self, PyObject *args);
static PyObject *llpy_wife (PyObject *self, PyObject *args);
static PyObject *llpy_nchildren (PyObject *self, PyObject *args);
#if !defined(DEADENDS)
static PyObject *llpy_nextfam (PyObject *self, PyObject *args);
static PyObject *llpy_prevfam (PyObject *self, PyObject *args);
#endif
static PyObject *llpy_firstchild (PyObject *self, PyObject *args);
static PyObject *llpy_lastchild (PyObject *self, PyObject *args);
static PyObject *llpy_children_f (PyObject *self, PyObject *args);
static PyObject *llpy_spouses_f (PyObject *self, PyObject *args);
#if !defined(DEADENDS)
static PyObject *llpy_choosechild_f (PyObject *self, PyObject *args);
static PyObject *llpy_choosespouse_f (PyObject *self, PyObject *args);
#endif

static void llpy_family_dealloc (PyObject *self);

static PyObject *llpy_marriage (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  NODE fam_node;
  NODE event = NULL;
  LLINES_PY_NODE *marr;

  fam_node = nztop (fam->llr_record);
  event = MARR (fam_node);

  if (! event)
    Py_RETURN_NONE;

  marr = PyObject_New (LLINES_PY_NODE, &llines_node_type);
  if (! marr)
    return NULL;		/* PyObject_New failed? -- out of memory?  */

  nrefcnt_inc(event);
  TRACK_NODE_REFCNT_INC(event);
  marr->lnn_node = event;
  marr->lnn_type = LLINES_TYPE_FAM;

  return ((PyObject *)marr);
}

/* llpy_husband (FAM) --> INDI

   Returns the first HUSB of the family.  None if there are none.  */

static PyObject *llpy_husband (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  NODE fam_node;
  NODE husb_node;
  LLINES_PY_RECORD *husb;
  CNSTRING key;

  fam_node = nztop (fam->llr_record);
  husb_node = HUSB (fam_node);
  if (! husb_node)
    /* family doesn't have a recorded husband */
    Py_RETURN_NONE;

  key = rmvat (nval (husb_node));
  if (! key)
    Py_RETURN_NONE;		/* it has a HUSB line, but no value -- no husband */

  husb = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! husb)
    return NULL;		/* PyObject_New failed -- out of memory? */

  husb->llr_record = key_to_irecord (key);
  husb->llr_type = LLINES_TYPE_INDI;

  return (PyObject *)husb;
}

/* llpy_wife (FAM) --> INDI

   Returns the first WIFE of the family.  None if there are none.  */

static PyObject *llpy_wife (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  NODE fam_node;
  NODE wife_node;
  LLINES_PY_RECORD *wife;
  CNSTRING key;

  fam_node = nztop (fam->llr_record);
  wife_node = WIFE (fam_node);
  if (! wife_node)
    /* family doesn't have a recorded wife */
    Py_RETURN_NONE;

  key = rmvat (nval (wife_node));
  if (! key)
    Py_RETURN_NONE;		/* it has a WIFE line, but not value -- no wife */

  wife = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! wife)
    return NULL;		/* PyObject_New failed -- out of memory? */

  wife->llr_record = key_to_irecord (key);
  wife->llr_type = LLINES_TYPE_INDI;

  return (PyObject *)wife;
}

/* llpy_nchilden (FAM) --> INTEGER

   Returns the number of children in the family.

   NOTE: sensitive to Lifelines GEDCOM format -- which puts CHIL links
   *LAST*.  If this changes, this breaks. */

static PyObject *llpy_nchildren (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  RECORD fam_record = fam->llr_record;
  int count = length_nodes (CHIL (nztop (fam_record)));

  return (Py_BuildValue ("i", count));
}

/* llpy_firstchild (FAM) --> INDI

   Returns the first child of FAM.  */

static PyObject *llpy_firstchild (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  NODE indi_node;
  LLINES_PY_RECORD *indi;
  CNSTRING key;

  indi_node = CHIL(nztop (fam->llr_record));

  if (! indi_node)
    Py_RETURN_NONE;		/* no children */

  key = rmvat (nval (indi_node));
  if (! key)
    Py_RETURN_NONE;		/* malformed CHIL line -- no value, no  children */

  indi = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! indi)
    {
      /* PyObject_New failed -- presumably out of memory or corruption
	 detected.  It should have signalled an exception... */
      return NULL;
    }
  indi->llr_record = key_to_irecord (key);
  indi->llr_type = LLINES_TYPE_INDI;

  return (PyObject *)indi;
}

/* llpy_lastchild (FAM) --> INDI

   Returns the last child of FAM.  */

static PyObject *llpy_lastchild (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  NODE indi_node;
  NODE prev_node;
  LLINES_PY_RECORD *indi;
  CNSTRING key;

  indi_node = CHIL (nztop (fam->llr_record));
  prev_node = indi_node;

  if (! indi_node)
    Py_RETURN_NONE;		/* no children */

  /* cycle through all remaining nodes, keeping most recent CHIL node */
  while (indi_node)
    {
      if (eqstr (ntag (indi_node), "CHIL"))
	prev_node = indi_node;
      indi_node = nsibling (indi_node);
    }
  key = rmvat (nval (prev_node));
  if (! key)
    Py_RETURN_NONE;		/* malformed CHIL line -- no value, no children */

  indi = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! indi)
    {
      /* PyObject_New failed -- presumably out of memory or corruption
	 detected.  It should have signalled an exception... */
      return NULL;
    }
  indi->llr_record = key_to_irecord (key);
  indi->llr_type = LLINES_TYPE_INDI;

  return (PyObject *)indi;
}

#if !defined(DEADENDS)
/* nextfam & prevfam are commented out until DeadEnds adds a 'highest
   family record number' variable */

/* llpy_nextfam (FAM) --> FAM

   Returns the next family (in key order) in the database.  */

static PyObject *llpy_nextfam (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  INT key_val = nzkeynum(fam->llr_record);

  if (key_val == 0)
    {
      /* unexpected internal error occurred -- raise an exception */
      PyErr_SetString(PyExc_SystemError, "nextfam: unable to determine RECORD's key");
      return NULL;
    }
  key_val = (long)xref_nextf ((INT)key_val);

  if (key_val == 0)
    Py_RETURN_NONE;		/* no more -- we have reached the end */

  fam = PyObject_New (LLINES_PY_RECORD, &llines_family_type);
  fam->llr_type = LLINES_TYPE_FAM;
  fam->llr_record = keynum_to_frecord (key_val);
  return (PyObject *)fam;
}

/* llpy_prevfam (FAM) --> FAM

   Returns the previous family (in key order) in the database.  */

static PyObject *llpy_prevfam (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  INT key_val = nzkeynum(fam->llr_record);

  if (key_val == 0)
    {
      /* unexpected internal error occurred -- raise an exception */
      PyErr_SetString(PyExc_SystemError, "prevfam: unable to determine RECORDs key");
      return NULL;
    }
  key_val = (long)xref_prevf ((INT)key_val);

  if (key_val == 0)
    Py_RETURN_NONE;		/* no more -- we have reached the end */

  fam = PyObject_New (LLINES_PY_RECORD, &llines_family_type);
  fam->llr_type = LLINES_TYPE_FAM;
  fam->llr_record = keynum_to_frecord (key_val);
  return (PyObject *)fam;
}
#endif

/* llpy_children_f (FAM) --> set of INDI

   Returns the set of children in the family.  */

static PyObject *llpy_children_f (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  RECORD fam = ((LLINES_PY_RECORD *)self)->llr_record;
  PyObject *output_set = PySet_New (NULL);

  if (! output_set)
    return NULL;

  FORCHILDREN_RECORD (fam, child)
    LLINES_PY_RECORD *py_record = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
    if (! py_record)
      {
	PySet_Clear (output_set);
	Py_DECREF (output_set);
	release_record (child);
	return NULL;
      }
    py_record->llr_record = child;
    py_record->llr_type = LLINES_TYPE_FAM;
    if (PySet_Add (output_set, (PyObject *)py_record) < 0)
      {
	PySet_Clear (output_set);
	Py_DECREF (output_set);
	Py_DECREF ((PyObject *)py_record);
	return NULL;
      }
  ENDCHILDREN_RECORD
  return (output_set);
}

/* llpy_spouses_f (FAM) --> set of INDI

   Returns the set of spouses in the family.  */

static PyObject *llpy_spouses_f (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  RECORD fam = ((LLINES_PY_RECORD *)self)->llr_record;
  PyObject *output_set = PySet_New (NULL);

  if (! output_set)
    return NULL;

  FORFAMSPOUSES_RECORD (fam, spouse)
    LLINES_PY_RECORD *py_record = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
    if (! py_record)
      {
	PySet_Clear (output_set);
	Py_DECREF (output_set);
	release_record (spouse);
	return NULL;
      }
    py_record->llr_record = spouse;
    py_record->llr_type = LLINES_TYPE_FAM;
    if (PySet_Add (output_set, (PyObject *)py_record) < 0)
      {
	PySet_Clear (output_set);
	Py_DECREF (output_set);
	Py_DECREF ((PyObject *)py_record);
	return NULL;
      }
  ENDFAMSPOUSES_RECORD
  return (output_set);
}

#if !defined(DEADENDS)		/* commented out until we have a user interface */
/* llpy_choosechild (FAM) --> INDI

   Figures out FAM's set of children and asks the user to choose one.
   Returns None if FAM has no children or if the user cancelled the
   operation. */

static PyObject *llpy_choosechild_f (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  NODE node = nztop (fam->llr_record);
  INDISEQ seq=0;
  RECORD record;
  LLINES_PY_RECORD *indi;

  if (! node)
    {
      /* unexpected internal error occurred -- raise an exception */
      PyErr_SetString(PyExc_SystemError, "choosechild: unable to find RECORD's top NODE");
      return NULL;
    }

  seq = fam_to_children (node);

  if (! seq || (length_indiseq (seq) < 1))
      Py_RETURN_NONE;	/* no children to choose from */

  record = choose_from_indiseq(seq, DOASK1, _(qSifonei), _(qSnotonei));
  remove_indiseq (seq);

  indi = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! indi)
    return NULL;

  indi->llr_type = LLINES_TYPE_INDI;
  indi->llr_record = record;

  return (PyObject *)indi;
}

static PyObject *llpy_choosespouse_f (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  NODE node = nztop (fam->llr_record);
  RECORD record;
  INDISEQ seq;
  LLINES_PY_RECORD *py_indi;

  seq = fam_to_spouses (node);
  if (! seq || (length_indiseq (seq) < 1))
    Py_RETURN_NONE;		/* no spouses for family */

  record = choose_from_indiseq (seq, DOASK1, _(qSifonei), _(qSnotonei));
  remove_indiseq (seq);
  if (! record)
    Py_RETURN_NONE;		/* user cancelled */

  py_indi = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! py_indi)
    return NULL;		/* no memory? */

  py_indi->llr_type = LLINES_TYPE_INDI;
  py_indi->llr_record = record;
  return ((PyObject *)py_indi);
}
#endif

#if !defined(DEADENDS)		/* For DeadEnds, sync does not make sense */
static PyObject *llpy_sync_fam (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *py_record = (LLINES_PY_RECORD *) self;
  RECORD record = py_record->llr_record;
  CNSTRING key = nzkey (record);
  String rawrec = 0;
  String msg = 0;
  int on_disk = 1;
  INT len;
  INT cnt;
  NODE old_tree = 0;
  NODE new_tree = 0;

  if (readonly)
    {
      PyErr_SetString (PyExc_PermissionError, "sync: database was opened read-only");
      return NULL;
    }

  if (! key)
    {
      PyErr_SetString (PyExc_SystemError, "sync: unable to determine record's key");
      return NULL;
    }
  new_tree = nztop (record);
  if (! new_tree)
    {
      PyErr_SetString (PyExc_SystemError, "sync: unable to find top of record");
      return NULL;
    }
  new_tree = copy_node_subtree (new_tree);

  rawrec = retrieve_raw_record (key, &len);
  if (! rawrec)
    on_disk = 0;
  else
    {
      ASSERT (old_tree = string_to_node (rawrec));
    }

  cnt = resolve_refn_links (new_tree);

  if (! valid_fam_tree (new_tree, &msg, old_tree))
    {
      PyErr_SetString (PyExc_SystemError, msg);
      stdfree (&rawrec);
      return NULL;
    }
  if (cnt > 0)
    {
      /* XXX unresolvable refn links -- existing code does nothing XXX */
    }
  if (equal_tree (old_tree, new_tree))
    {
      /* no modifications -- return success */
      stdfree (&rawrec);
      Py_RETURN_TRUE;
    }

  if (on_disk)
    replace_fam (old_tree, new_tree);
#if 0				/* XXX */
  else
    add_new_indi_to_db (record);
#endif
  stdfree (&rawrec);
  Py_RETURN_TRUE;
}
#endif

static void llpy_family_dealloc (PyObject *self)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_family_dealloc entry: self %p\n refcnt %ld",
	       (void *)self, Py_REFCNT (self));
    }
  release_record (fam->llr_record);
  fam->llr_record = 0;
  fam->llr_type = 0;
  Py_TYPE(self)->tp_free (self);
}

static struct PyMethodDef Lifelines_Family_Methods[] =
  {
   { "marriage",	llpy_marriage, METH_NOARGS,
     "(FAM).marriage --> NODE: First marriage event of FAM; None if no event is found." },
   { "husband",		llpy_husband, METH_NOARGS,
     "(FAM).husband --> INDI: First husband of FAM; None if no person in the role." },
   { "wife",		llpy_wife, METH_NOARGS,
     "(FAM).wife --> INDI: FIrst wife of FAM; None if no person in the role." },
   { "nchildren",	llpy_nchildren, METH_NOARGS,
     "(FAM).nchildren -> INTEGER: number of children in FAM." },
   { "firstchild",	llpy_firstchild, METH_NOARGS,
     "(FAM).firstchild -> INDI: first child of FAM; None if no children." },
   { "lastchild",	llpy_lastchild, METH_NOARGS,
     "(FAM).lastchild -> INDI: last child of FAM; None if no children." },
   { "key", (PyCFunction)_llpy_key, METH_VARARGS | METH_KEYWORDS,
     "(FAM).key([strip_prefix]) --> STRING.  Returns the database key of the record.\n\
If STRIP_PREFIX is True (default: False), the non numeric prefix is stripped." },
#if !defined(DEADENDS)
   { "nextfam",		llpy_nextfam, METH_NOARGS,
     "(FAM).nextfam -> FAM: next family in database after FAM (in key order)" },
   { "prevfam",		llpy_prevfam, METH_NOARGS,
     "(FAM).prevfam -> FAM: previous family in database before FAM (in key order)" },
#endif
   { "children",	llpy_children_f, METH_NOARGS,
     "(FAM).children -> set of children in the family" },
   { "spouses",		llpy_spouses_f, METH_NOARGS,
     "(FAM).spouses -> set of spouses in the family" },

#if !defined(DEADENDS)
   { "choosechild",	llpy_choosechild_f, METH_NOARGS,
     "(FAM).choosechild --> INDI.  Select and return child of family\n\
 through user interface. Returns None if family has no children." },

   { "choosespouse",	llpy_choosespouse_f, METH_NOARGS,
     "(FAM).choosespouse(void) --> INDI.  Select and return spouse of family\n\
through user interface.  Returns None if family has no spouses or user cancels." },
#endif

   { "top_node", (PyCFunction)_llpy_top_node, METH_NOARGS,
     "top_node(void) --> NODE.  Returns the top of the NODE tree associated with the RECORD." },

#if !defined(DEADENDS)
   { "sync", (PyCFunction)llpy_sync_fam, METH_NOARGS,
     "sync(void) --> BOOLEAN.  Writes modified FAM to database.\n\
Returns success or failure."},
#endif

   { NULL, 0, 0, NULL }		/* sentinel */
  };

PyTypeObject llines_family_type =
  {
   PyVarObject_HEAD_INIT(NULL, 0)
   .tp_name = "llines.Family",
   .tp_doc = "Lifelines GEDCOM Family Record",
   .tp_basicsize = sizeof (LLINES_PY_RECORD),
   .tp_itemsize = 0,
   .tp_flags = Py_TPFLAGS_DEFAULT,
   .tp_new = PyType_GenericNew,
   .tp_dealloc = llpy_family_dealloc,
   .tp_hash = llines_record_hash,
   .tp_richcompare = llines_record_richcompare,
   .tp_methods = Lifelines_Family_Methods,
  };

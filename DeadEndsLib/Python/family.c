/* family.c -- family functions.

   These are the bulk of the functions that are documented in the
   'Family functions' section of the manual.  */

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
#include "database.h"
#include "recordindex.h"	/* RecordIndexEl */
#include "nodeutils.h"		/* equal_tree */
#include "py-messages.h"
#include "ll-node.h"
#include "sequence.h"
#include "ui.h"
#include "xref.h"
#include "ll-sequence.h"
#include "ask.h"

#include "python-to-c.h"
#include "types.h"
#include "py-set.h"
#include "family.h"

/* forward references */

static PyObject *llpy_marriage (PyObject *self, PyObject *args);
static PyObject *llpy_husband (PyObject *self, PyObject *args);
static PyObject *llpy_wife (PyObject *self, PyObject *args);
static PyObject *llpy_nchildren (PyObject *self, PyObject *args);
static PyObject *llpy_nextfam (PyObject *self, PyObject *args);
static PyObject *llpy_prevfam (PyObject *self, PyObject *args);
static PyObject *llpy_firstchild (PyObject *self, PyObject *args);
static PyObject *llpy_lastchild (PyObject *self, PyObject *args);
static PyObject *llpy_children_f (PyObject *self, PyObject *args);
static PyObject *llpy_spouses_f (PyObject *self, PyObject *args);
static PyObject *llpy_choosechild_f (PyObject *self, PyObject *args);
static PyObject *llpy_choosespouse_f (PyObject *self, PyObject *args);

static void llpy_family_dealloc (PyObject *self);

static PyObject *llpy_marriage (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  GNode *fam_node;
  GNode *event = NULL;
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
  marr->lnn_database = fam->llr_database;
  marr->lnn_type = LLINES_TYPE_FAM;

  return ((PyObject *)marr);
}

/* llpy_husband (FAM) --> INDI

   Returns the first HUSB of the family.  None if there are none.  */

static PyObject *llpy_husband (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  Database *database = fam->llr_database;
  GNode *fam_node;
  GNode *husb_node;
  LLINES_PY_RECORD *husb;
  CString key;

  fam_node = nztop (fam->llr_record);
  husb_node = HUSB (fam_node);
  if (! husb_node)
    /* family doesn't have a recorded husband */
    Py_RETURN_NONE;

  key = nval (husb_node);
  if (! key)
    Py_RETURN_NONE;		/* it has a HUSB line, but no value -- no husband */

  husb = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! husb)
    return NULL;		/* PyObject_New failed -- out of memory? */

  husb->llr_record = keyToPersonRecord (key, database);
  husb->llr_database = database;
  husb->llr_type = LLINES_TYPE_INDI;

  return (PyObject *)husb;
}

/* llpy_wife (FAM) --> INDI

   Returns the first WIFE of the family.  None if there are none.  */

static PyObject *llpy_wife (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  Database *database = fam->llr_database;
  GNode *fam_node;
  GNode *wife_node;
  LLINES_PY_RECORD *wife;
  CString key;

  fam_node = nztop (fam->llr_record);
  wife_node = WIFE (fam_node);
  if (! wife_node)
    /* family doesn't have a recorded wife */
    Py_RETURN_NONE;

  key = nval (wife_node);
  if (! key)
    Py_RETURN_NONE;		/* it has a WIFE line, but not value -- no wife */

  wife = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! wife)
    return NULL;		/* PyObject_New failed -- out of memory? */

  wife->llr_record = keyToPersonRecord (key, database);
  wife->llr_database = database;
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
  RecordIndexEl *fam_record = fam->llr_record;
  int count = gNodesLength (CHIL (nztop (fam_record)));

  return (Py_BuildValue ("i", count));
}

/* llpy_firstchild (FAM) --> INDI

   Returns the first child of FAM.  */

static PyObject *llpy_firstchild (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  Database *database = fam->llr_database;
  GNode *indi_node;
  LLINES_PY_RECORD *indi;
  CString key;

  indi_node = CHIL(nztop (fam->llr_record));

  if (! indi_node)
    Py_RETURN_NONE;		/* no children */

  key = nval (indi_node);
  if (! key)
    Py_RETURN_NONE;		/* malformed CHIL line -- no value, no  children */

  indi = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! indi)
    {
      /* PyObject_New failed -- presumably out of memory or corruption
	 detected.  It should have signalled an exception... */
      return NULL;
    }
  indi->llr_record = keyToPersonRecord (key, database);
  indi->llr_database = database;
  indi->llr_type = LLINES_TYPE_INDI;

  return (PyObject *)indi;
}

/* llpy_lastchild (FAM) --> INDI

   Returns the last child of FAM.  */

static PyObject *llpy_lastchild (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  Database *database = fam->llr_database;
  GNode *indi_node;
  GNode *prev_node;
  LLINES_PY_RECORD *indi;
  CString key;

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
  key = nval (prev_node);
  if (! key)
    Py_RETURN_NONE;		/* malformed CHIL line -- no value, no children */

  indi = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! indi)
    {
      /* PyObject_New failed -- presumably out of memory or corruption
	 detected.  It should have signalled an exception... */
      return NULL;
    }
  indi->llr_record = keyToPersonRecord (key, database);
  indi->llr_database = database;
  indi->llr_type = LLINES_TYPE_INDI;

  return (PyObject *)indi;
}

/* llpy_nextfam (FAM) --> FAM

   Returns the next family (in key order) in the database.  */

static PyObject *llpy_nextfam (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  CString key = nzkey(fam->llr_record);
  Database *database = fam->llr_database;
  RecordIndexEl *new;

  if (! key)
    {
      /* unexpected internal error occurred -- raise an exception */
      PyErr_SetString(PyExc_SystemError, "nextfam: unable to determine RECORD's key");
      return NULL;
    }
  new = getNextFamilyRecord (key, fam->llr_database);

  if (! new)
    Py_RETURN_NONE;		/* no more -- we have reached the end */

  fam = PyObject_New (LLINES_PY_RECORD, &llines_family_type);
  fam->llr_type = LLINES_TYPE_FAM;
  fam->llr_database = database;
  fam->llr_record = new;
  return (PyObject *)fam;
}

/* llpy_prevfam (FAM) --> FAM

   Returns the previous family (in key order) in the database.  */

static PyObject *llpy_prevfam (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  CString key = nzkey(fam->llr_record);
  Database *database = fam->llr_database;
  RecordIndexEl *new;

  if (! key)
    {
      /* unexpected internal error occurred -- raise an exception */
      PyErr_SetString(PyExc_SystemError, "prevfam: unable to determine RECORDs key");
      return NULL;
    }
  new = getPreviousFamilyRecord (key, fam->llr_database);
  if (! new)
    Py_RETURN_NONE;		/* no more -- we have reached the beginning */

  fam = PyObject_New (LLINES_PY_RECORD, &llines_family_type);
  fam->llr_type = LLINES_TYPE_FAM;
  fam->llr_database = database;
  fam->llr_record = new;
  return (PyObject *)fam;
}

/* llpy_children_f (FAM) --> set of INDI

   Returns the set of children in the family.  */

static PyObject *llpy_children_f (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  RecordIndexEl *fam = ((LLINES_PY_RECORD *)self)->llr_record;
  Database *database = ((LLINES_PY_RECORD *)self)->llr_database;
  PyObject *output_set = PySet_New (NULL);

  if (! output_set)
    return NULL;

  FORCHILDREN_RECORD (fam, child, database)
    LLINES_PY_RECORD *py_record = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
    if (! py_record)
      {
	PySet_Clear (output_set);
	Py_DECREF (output_set);
	releaseRecord (child);
	return NULL;
      }
    py_record->llr_record = child;
    py_record->llr_database = database;
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
  RecordIndexEl *fam = ((LLINES_PY_RECORD *)self)->llr_record;
  Database *database = ((LLINES_PY_RECORD *)self)->llr_database;
  PyObject *output_set = PySet_New (NULL);

  if (! output_set)
    return NULL;

  FORFAMSPOUSES_RECORD (fam, spouse, database)
    LLINES_PY_RECORD *py_record = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
    if (! py_record)
      {
	PySet_Clear (output_set);
	Py_DECREF (output_set);
	releaseRecord (spouse);
	return NULL;
      }
    py_record->llr_record = spouse;
    py_record->llr_database = database;
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

/* llpy_choosechild (FAM) --> INDI

   Figures out FAM's set of children and asks the user to choose one.
   Returns None if FAM has no children or if the user cancelled the
   operation. */

static PyObject *llpy_choosechild_f (PyObject *self, PyObject *args ATTRIBUTE_UNUSED)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;
  GNode *node = nztop (fam->llr_record);
  Database *database = fam->llr_database;
  Sequence *seq=0;
  RecordIndexEl *record;
  LLINES_PY_RECORD *indi;

  if (! node)
    {
      /* unexpected internal error occurred -- raise an exception */
      PyErr_SetString(PyExc_SystemError, "choosechild: unable to find RECORD's top NODE");
      return NULL;
    }

  seq = familyToChildren (node, database->recordIndex);

  if (! seq || (seq->block.length < 1))
      Py_RETURN_NONE;	/* no children to choose from */

  record = chooseFromSequence(seq, DOASK1, _(qSifonei), _(qSnotonei));
  deleteSequence (seq);

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
  GNode *node = nztop (fam->llr_record);
  RecordIndexEl *record;
  Sequence *seq;
  LLINES_PY_RECORD *py_indi;

  seq = familyToSpouses (node, fam->llr_database);
  if (! seq || (seq->block.length < 1))
    Py_RETURN_NONE;		/* no spouses for family */

  record = chooseFromSequence (seq, DOASK1, _(qSifonei), _(qSnotonei));
  deleteSequence (seq);
  if (! record)
    Py_RETURN_NONE;		/* user cancelled */

  py_indi = PyObject_New (LLINES_PY_RECORD, &llines_individual_type);
  if (! py_indi)
    return NULL;		/* no memory? */

  py_indi->llr_type = LLINES_TYPE_INDI;
  py_indi->llr_record = record;
  return ((PyObject *)py_indi);
}

static void llpy_family_dealloc (PyObject *self)
{
  LLINES_PY_RECORD *fam = (LLINES_PY_RECORD *) self;

  if (llpy_debug)
    {
      fprintf (stderr, "llpy_family_dealloc entry: self %p\n refcnt %ld",
	       (void *)self, Py_REFCNT (self));
    }
  releaseRecord (fam->llr_record);
  fam->llr_record = 0;
  fam->llr_database = 0;
  fam->llr_type = 0;
  Py_TYPE(self)->tp_free (self);
}

static struct PyMethodDef Lifelines_Family_Methods[] =
  {
   { "marriage",	llpy_marriage, METH_NOARGS,
     "(FAM).marriage(void) --> NODE: First marriage event of FAM; None if no event is found." },
   { "husband",		llpy_husband, METH_NOARGS,
     "(FAM).husband(void) --> INDI: First husband of FAM; None if no person in the role." },
   { "wife",		llpy_wife, METH_NOARGS,
     "(FAM).wife(void) --> INDI: FIrst wife of FAM; None if no person in the role." },
   { "nchildren",	llpy_nchildren, METH_NOARGS,
     "(FAM).nchildren(void) -> INTEGER: number of children in FAM." },
   { "firstchild",	llpy_firstchild, METH_NOARGS,
     "(FAM).firstchild(void) -> INDI: first child of FAM; None if no children." },
   { "lastchild",	llpy_lastchild, METH_NOARGS,
     "(FAM).lastchild(void) -> INDI: last child of FAM; None if no children." },
   { "key", (PyCFunction)_llpy_key, METH_VARARGS | METH_KEYWORDS,
     "(FAM).key([strip_prefix]) --> STRING.  Returns the database key of the record.\n\n\
If STRIP_PREFIX is True (default: False), the non numeric prefix is stripped." },
   { "nextfam",		llpy_nextfam, METH_NOARGS,
     "(FAM).nextfam(void) -> FAM: next family in database after FAM (in key order)" },
   { "prevfam",		llpy_prevfam, METH_NOARGS,
     "(FAM).prevfam(void) -> FAM: previous family in database before FAM (in key order)" },
   { "children",	llpy_children_f, METH_NOARGS,
     "(FAM).children(void) -> set of children in the family" },
   { "spouses",		llpy_spouses_f, METH_NOARGS,
     "(FAM).spouses(void) -> set of spouses in the family" },

   { "choosechild",	llpy_choosechild_f, METH_NOARGS,
     "(FAM).choosechild(void) --> INDI.  Select and return child of family\n\
 through user interface. Returns None if family has no children." },

   { "choosespouse",	llpy_choosespouse_f, METH_NOARGS,
     "(FAM).choosespouse(void) --> INDI.  Select and return spouse of family\n\
through user interface.  Returns None if family has no spouses or user cancels." },

   { "top_node", (PyCFunction)_llpy_top_node, METH_NOARGS,
     "(FAM).top_node(void) --> NODE.  Returns the top of the NODE tree associated with the RECORD." },

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

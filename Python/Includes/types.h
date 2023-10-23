/* requires PY_SSIZE_T_CLEAN to be defined and <Python.h> to be
   included first */

typedef struct {
  PyObject_HEAD
  int lnn_type;
  NODE lnn_node;		/* Lifelines GEDCOM generic NODE */
} LLINES_PY_NODE;

/* type values for records and nodes -- will likely be augmented in
   the future.  And might become numerical rather than mnemonic.  The
   'OTHR' type should disappear -- it is a catchall -- and be broken
   out into the types contained therein -- SUBM, SNOTE, OBJE, REPO. */

#define LLINES_TYPE_INDI	'I' /* individual */
#define LLINES_TYPE_FAM		'F' /* family */
#define LLINES_TYPE_SOUR	'S' /* source */
#define LLINES_TYPE_EVEN	'E' /* event */
#define LLINES_TYPE_OTHR	'X' /* other */

typedef struct {
  PyObject_HEAD
  int llr_type;
  RECORD llr_record;		/* Lifelines GEDCOM generic RECORD */
} LLINES_PY_RECORD;

typedef struct {
  PyObject_HEAD
  /* XXX insert fields here that are useful for database 'types' XXX */
} LLINES_PY_DATABASE;

/* the 'iter' functions for each RECORD type (INDI, FAM...) provide an
   instance of this type with the li_type field set to the RECORD type
   and the li_current field set to zero.

   The tp_iternext function for this type then examines the li_type
   field to know which xref_next* function to call, supplying the
   li_current value.  The returned value is then both stored in
   li_current and converted to a RECORD that is returned to the
   caller.  */

typedef struct {
  PyObject_HEAD
  int li_type;			/* the type being iterated over, NOT *our* type */
#if defined(DEADENDS)
  int li_bucket_ndx;	/* arguments to supply to {first|next}InHashTable */
  int li_element_ndx;
#else
  int li_current;		/* argument to supply to xref_next* function */
#endif
} LLINES_PY_RECORD_ITER;

/* iter functions for NODEs use a different underlying type */

typedef struct {
  PyObject_HEAD
  NODE ni_top_node; /* top of the node tree that we are iterating over */
  NODE ni_cur_node; /* most recently visited node in the node tree */
  char *ni_tag;	    /* tag to search for if we are tag specific */
  int ni_type;	    /* type of iteration -- whole tree or just children */
  int ni_level;	    /* how far are we from the top of the tree */
} LLINES_PY_NODEITER;

#define ITER_CHILDREN	1
#define ITER_TRAVERSE	2

extern PyTypeObject llines_database_type;

/* various GEDCOM record types */

extern PyTypeObject llines_event_type; /* Lifelines extension */

extern PyTypeObject llines_family_type;
extern PyTypeObject llines_individual_type;
extern PyTypeObject llines_other_type; /* catchall -- should be broken out */
extern PyTypeObject llines_source_type;

extern PyTypeObject llines_record_iter_type;
extern PyTypeObject llines_node_iter_type;

/* generic GEDCOM record type -- not sure if we need this */

extern PyTypeObject llines_record_type;

/* GEDCOM node */
extern PyTypeObject llines_node_type;

/* functions for comparing two node or record Python objects */

extern PyObject *llines_record_richcompare (PyObject *self, PyObject *other, int op);
extern PyObject *llines_node_richcompare (PyObject *self, PyObject *other, int op);
extern Py_hash_t llines_record_hash (PyObject *obj);

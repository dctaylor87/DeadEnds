/* python-to-c -- private interfaces

   these are functions that the Python interpreter can call -- these
   functions extend the intrepreter.  These functions are currently
   non-static as the expectation is that they will be spread over
   multiple files.  If that changes, this header might be eliminated
   and static variants of the declarations folded into the appropriate
   C file.  */

PyObject *llpy_chooseindi (PyObject *self, PyObject *args);
PyObject *llpy_choosesubset (PyObject *self, PyObject *args);

PyObject *llpy_choosespouse (PyObject *self, PyObject *args);

PyObject *llpy_gengedcomstrong (PyObject *self, PyObject *args);
PyObject *llpy_program (PyObject *self, PyObject *args);

extern PyObject *Lifelines_Module;
extern void llpy_iter_init (void);
extern void llpy_user_init (void);
extern void llpy_set_init (void);
extern void llpy_database_init (void);
extern void llpy_person_init (void);
extern void llpy_nodes_init (void);
extern void llpy_records_init (void);
extern void llpy_event_init (void);
extern void llpy_export_init (void);

/* XXX TODO: make this a bitmask XXX */
extern int llpy_debug;

extern PyObject *_llpy_key (PyObject *self, PyObject *args, PyObject *kw);
extern PyObject *_llpy_top_node (PyObject *self, PyObject *args);
extern RECORD __llpy_key_to_record (CString key, int *int_type);

extern int _py_traverse_nodes (GNode *node, int level,
			       int (*func)(GNode *, int, void *),
			       void *extra);

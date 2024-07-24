/* event.h -- used for communicating between node.c and event.c */

extern PyObject *llpy_date (PyObject *self, PyObject *args);
extern PyObject *llpy_place (PyObject *self, PyObject *args);
extern PyObject *llpy_year (PyObject *self, PyObject *args);
extern PyObject *llpy_long (PyObject *self, PyObject *args);
extern PyObject *llpy_short (PyObject *self, PyObject *args);
extern PyObject *llpy_stddate_node  (PyObject *self, PyObject *args);
extern PyObject *llpy_complexdate_node (PyObject *self, PyObject *args);

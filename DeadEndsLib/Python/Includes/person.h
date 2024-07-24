/* person.h -- public interfaces

   Increasingly these functions declarations are moving into person.c
   and being made static -- the functions are being made methods of
   the type.  */

/* returns set of families */
PyObject *llpy_indi_families (PyObject *self, PyObject *args);

#ifndef OCTSQLN_PY_RECORD_SET_OBJ
#define OCTSQLN_PY_RECORD_SET_OBJ

#include "core.h"

int RecordSetType_init(RecordObject *self, PyObject *args, PyObject *kwargs);

PyTypeObject* RecordSetType_get();

PyObject* get_record_set_obj_attr(PyObject *self, PyObject *args);

#endif // OCTSQLN_PY_RECORD_SET_OBJ
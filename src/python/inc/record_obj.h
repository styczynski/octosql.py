#ifndef OCTSQLN_PY_RECORD_OBJ
#define OCTSQLN_PY_RECORD_OBJ

#include "core.h"

int RecordType_init(RecordObject *self, PyObject *args, PyObject *kwargs);

PyObject* get_record_obj_attr(PyObject *self, PyObject *args);

PyObject* get_query_record_dict(int appID, int parseID, int recordID);

PyTypeObject* RecordType_get();

#endif // OCTSQLN_PY_RECORD_OBJ
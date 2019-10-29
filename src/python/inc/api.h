#ifndef OCTSQLN_PY_API
#define OCTSQLN_PY_API

#include "../inc/core.h"

PyObject* init(PyObject *self, PyObject *args);

PyObject* new_instance(PyObject *self, PyObject *args);

PyObject* create_record_native_wrapper(int appID, int parseID, int recordID);

PyObject* create_fields_list(int appID, int parseID, int recordID);

PyObject* create_new_parse(PyObject *self, PyObject *args);

PyObject* parse(PyObject *self, PyObject *args);

PyObject* plan(PyObject *self, PyObject *args);

PyObject* get_query_record_field_value(int appID, int parseID, int recordID, int fieldID);


PyObject* get_query_record_set_val(PyObject *self, PyObject *args);

PyObject* get_query_record_val(PyObject *self, PyObject *args);

PyObject* get_query_results_obj(int appID, int parseID);

PyObject* run(PyObject *self, PyObject *args);

#endif // OCTSQLN_PY_API
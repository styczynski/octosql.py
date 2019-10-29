#ifndef OCTSQLN_PY_CUSTOM_STORAGE
#define OCTSQLN_PY_CUSTOM_STORAGE

#include <Python.h>
#include "../../custom_storage/custom_storage.hpp"

PyObject* create_native_source(PyObject *self, PyObject *args);

NativeSourceRecord octosql_register_native_source_indirect_impl(int ptr);

#endif // OCTSQLN_PY_CUSTOM_STORAGE
#include "../inc/core.h"
#include "../inc/api.h"
#include "../inc/custom_storage.h"

PyMethodDef method_table[] = {
    {"init", (PyCFunction) init, METH_VARARGS, "Method docstring"},
    {"new_instance", (PyCFunction) new_instance, METH_VARARGS, "Method docstring"},
    {"create_new_parse", (PyCFunction) create_new_parse, METH_VARARGS, "Method docstring"},
    {"parse", (PyCFunction) parse, METH_VARARGS, "Method docstring"},
    {"plan", (PyCFunction) plan, METH_VARARGS, "Method docstring"},
    {"run", (PyCFunction) run, METH_VARARGS, "Method docstring"},
    {"create_native_source", (PyCFunction) create_native_source, METH_VARARGS, "Method docstring"},
    {NULL, NULL, 0, NULL} // Sentinel value ending the table
};

#ifdef PY3K
PyModuleDef octosql_py_native_module = {
    PyModuleDef_HEAD_INIT,
    "octosql_py_native", // Module name
    "This is the module docstrings",
    -1,   // Optional size of the module state memory
    method_table,
    NULL, // Optional slot definitions
    NULL, // Optional traversal function
    NULL, // Optional clear function
    NULL  // Optional module deallocation function
};

PyMODINIT_FUNC PyInit_octosql_py_native(void) {
    module = PyModule_Create(&octosql_py_native_module);
    if (PyType_Ready(&RecordType) < 0)
        return NULL;
    if (PyType_Ready(&RecordSetType) < 0)
        return NULL;

    if (module == NULL)
        return NULL;

    if (PyModule_AddObject(module, "Record", (PyObject *) &RecordType) < 0) {
        return NULL;
    }
    if (PyModule_AddObject(module, "RecordSet", (PyObject *) &RecordSetType) < 0) {
        return NULL;
    }

    return module;
}
#else
// module initializer for python2
PyMODINIT_FUNC initoctosql_py_native(void) {
    module = Py_InitModule3("octosql_py_native", method_table, "This is the module docstrings");
    if (PyType_Ready(RecordType_get()) < 0)
        return;
    if (PyType_Ready(RecordSetType_get()) < 0)
        return;

    if (module == NULL)
        return;

    if (PyModule_AddObject(module, "Record", (PyObject *) RecordType_get()) < 0) {
        return;
    }
    if (PyModule_AddObject(module, "RecordSet", (PyObject *) RecordSetType_get()) < 0) {
        return;
    }

    return;
}

#include "./custom_storage.cpp"
#include "./parse_python_record.cpp"
#include "./record_obj.cpp"
#include "./record_set_obj.cpp"
#include "./api.cpp"

#endif
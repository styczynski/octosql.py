#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <iostream>
#include "../../libs/libgooctosql.h"
#include "./helper.h"

PyObject* module = NULL;

typedef struct {
    int appID;
    int parseID;
    int recordID;
} RecordObjectCapsule;

void RecordObjectCapsule_destroy(PyObject *pycap) {
    RecordObjectCapsule *cap;
    cap = (RecordObjectCapsule*) PyCapsule_GetPointer(pycap, "NATIVE_RECORD");
    if (cap == NULL) {
         return;
    }

    free(cap);
}

static PyObject* get_query_record_field_value(int appID, int parseID, int recordID, int fieldID);

typedef struct {
    PyObject_HEAD
    PyObject* pycap;
    /* Type-specific fields go here. */
} RecordObject;

typedef struct {
    PyObject_HEAD
    PyObject* pycap;
    /* Type-specific fields go here. */
} RecordSetObject;

static PyObject* init(PyObject *self, PyObject *args) {
    const int res = octosql_init();
    if (res != 0) {
        SET_GO_ERR(octosql_get_error());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* new_instance(PyObject *self, PyObject *args) {
    char* yamlConfiguration;
    if (!PyArg_ParseTuple(args, "s", &yamlConfiguration)) {
        return NULL;
    }
    const int id = octosql_new_instance(strToGo(yamlConfiguration));
    return PyLong_FromLong(id);
}

static PyObject* get_query_record_dict(int appID, int parseID, int recordID) {
    PyObject *record = PyDict_New();
    const int fieldsCount = octosql_get_record_fields_count(appID, parseID, recordID);
    for (int i=0; i<fieldsCount; ++i) {
        PyObject* fieldVal = get_query_record_field_value(appID, parseID, recordID, i);
        if (fieldVal != NULL) {
            char* fieldName = octosql_get_record_field_name(appID, parseID, recordID, i);
            PyDict_SetItemString(record, fieldName, fieldVal);
        }
    }

    return record;
}

static PyObject* create_record_native_wrapper(int appID, int parseID, int recordID) {
    // dict is a borrowed reference.
    PyObject* dict = PyModule_GetDict(module);
    if (dict == nullptr) {
    PyErr_Print();
    std::cout << "Fails to get the dictionary.\n";
    return NULL;
    }

    // Builds the name of a callable class
    PyObject* python_class = PyDict_GetItemString(dict, "Record");
    if (python_class == nullptr) {
    PyErr_Print();
    std::cout << "Fails to get the Python class.\n";
    return NULL;
    }

    PyObject* object;
    // Creates an instance of the class
    if (PyCallable_Check(python_class)) {
    object = PyObject_CallObject(python_class, nullptr);
    } else {
    std::cout << "Cannot instantiate the Python class" << std::endl;
    return NULL;
    }

    RecordObjectCapsule* cap = (RecordObjectCapsule*) malloc(sizeof(RecordObjectCapsule));
    cap->appID = appID;
    cap->parseID = parseID;
    cap->recordID = recordID;

    PyObject* pycap = PyCapsule_New((void *)cap, "NATIVE_RECORD", RecordObjectCapsule_destroy);
    ((RecordObject*) object)->pycap = pycap;

    return object;
}

static PyObject* create_fields_list(int appID, int parseID, int recordID) {
    const int fieldsCount = octosql_get_record_fields_count(appID, parseID, recordID);
    PyObject *fieldsList = PyList_New(fieldsCount);
    for (int i=0; i<fieldsCount; ++i) {
        const char* fieldName = octosql_get_record_field_name(appID, parseID, recordID, i);
        PyObject* object = PyUnicode_FromString(fieldName);
        PyList_SetItem(fieldsList, i, object);
    }
    return fieldsList;
}

static PyObject* get_record_set_obj_attr(PyObject *self, PyObject *args) {
    if(PyUnicode_Check(args)) {
        Py_ssize_t size;
        const char *ptr = PyUnicode_AsUTF8AndSize(args, &size);

        PyObject* pycap = ((RecordObject*) self)->pycap;

        RecordObjectCapsule *cap;
        cap = (RecordObjectCapsule*) PyCapsule_GetPointer(pycap, "NATIVE_RECORD");
        const int appID = cap->appID;
        const int parseID = cap->parseID;
        if (strcmp(ptr, "count") == 0) {
            return PyLong_FromLong(octosql_get_records_count(appID, parseID));
        } else if (strcmp(ptr, "records") == 0) {
            const int recordsCount = octosql_get_records_count(appID, parseID);

            PyObject *recordsList = PyList_New(recordsCount);
            for(int i=0;i<recordsCount;++i) {
                PyList_SetItem(recordsList, i, create_record_native_wrapper(appID, parseID, i));
            }
            return recordsList;
        } else if (strcmp(ptr, "fields") == 0) {
            const int recordsCount = octosql_get_records_count(appID, parseID);
            if (recordsCount > 0) {
                return create_fields_list(appID, parseID, 0);
            }
            return PyList_New(0);
        } else if (strcmp(ptr, "values") == 0) {
            const int recordsCount = octosql_get_records_count(appID, parseID);
            PyObject *recordsList = PyList_New(recordsCount);
            for (int i=0; i<recordsCount; ++i) {
                PyList_SetItem(recordsList, i, get_query_record_dict(appID, parseID, i));
            }
            return recordsList;
        }

        Py_RETURN_NONE;
    }
    return NULL;
}

static PyObject* get_record_obj_attr(PyObject *self, PyObject *args) {
    if(PyUnicode_Check(args)) {
        Py_ssize_t size;
        const char *ptr = PyUnicode_AsUTF8AndSize(args, &size);

        PyObject* pycap = ((RecordObject*) self)->pycap;

        RecordObjectCapsule *cap;
        cap = (RecordObjectCapsule*) PyCapsule_GetPointer(pycap, "NATIVE_RECORD");
        const int appID = cap->appID;
        const int parseID = cap->parseID;
        const int recordID = cap->recordID;
        if (strcmp(ptr, "fields") == 0) {
            return create_fields_list(appID, parseID, recordID);
        } else if(strcmp(ptr, "values") == 0) {
            return get_query_record_dict(appID, parseID, recordID);
        }

        Py_RETURN_NONE;
    }
    return NULL;
}

static PyObject* create_new_parse(PyObject *self, PyObject *args) {
    return PyLong_FromLong(octosql_create_new_parse());
}

static PyObject* parse(PyObject *self, PyObject *args) {
    int appID, parseID;
    char* input;

    if (!PyArg_ParseTuple(args, "lls", &appID, &parseID, &input)) {
        return NULL;
    }

    octosql_parse(appID, parseID, strToGo(input));
    Py_RETURN_NONE;
}

static PyObject* plan(PyObject *self, PyObject *args) {
    int appID, parseID;

    if (!PyArg_ParseTuple(args, "ll", &appID, &parseID)) {
        return NULL;
    }

    octosql_plan(appID, parseID);
    Py_RETURN_NONE;
}

static PyObject* get_query_record_field_value(int appID, int parseID, int recordID, int fieldID) {
    PyObject* fieldVal = NULL;
    const int fieldType = octosql_get_record_field_type(appID, parseID, recordID, fieldID);
    switch (fieldType) {
        case 0:
            fieldVal = PyLong_FromLong(octosql_get_record_field_as_int(appID, parseID, recordID, fieldID));
            break;
        case 3:
            fieldVal = PyUnicode_FromString(octosql_get_record_field_as_string(appID, parseID, recordID, fieldID));
            break;
        case 6:
            fieldVal = PyFloat_FromDouble(octosql_get_record_field_as_float(appID, parseID, recordID, fieldID));
            break;
        default:
            std::cout << "Type other = " << fieldType << "\n";
            break;
    }
    if (fieldVal != NULL) {
        return fieldVal;
    }
    Py_RETURN_NONE;
}


static PyObject* get_query_record_set_val(PyObject *self, PyObject *args) {
    PyObject* pycap = ((RecordObject*) self)->pycap;

    RecordObjectCapsule *cap;
    cap = (RecordObjectCapsule*) PyCapsule_GetPointer(pycap, "NATIVE_RECORD");
    const int appID = cap->appID;
    const int parseID = cap->parseID;

    int recordID = 0;

    if (PyLong_Check(args)) {
        recordID = (int) PyLong_AsLong(args);
    }

    return create_record_native_wrapper(appID, parseID, recordID);
}

static PyObject* get_query_record_val(PyObject *self, PyObject *args) {
    PyObject* pycap = ((RecordObject*) self)->pycap;

    RecordObjectCapsule *cap;
    cap = (RecordObjectCapsule*) PyCapsule_GetPointer(pycap, "NATIVE_RECORD");
    const int appID = cap->appID;
    const int parseID = cap->parseID;
    const int recordID = cap->recordID;

    int fieldID = 0;

    if (PyLong_Check(args)) {
        fieldID = (int) PyLong_AsLong(args);
    } else if(PyUnicode_Check(args)) {
        Py_ssize_t size;
        const char *ptr = PyUnicode_AsUTF8AndSize(args, &size);
        GoString goStr = {
            .p = ptr,
            .n = size,
        };

        fieldID = octosql_get_record_field_id(appID, parseID, recordID, goStr);
    }

    return get_query_record_field_value(appID, parseID, recordID, fieldID);
}

static PyObject* get_query_results_obj(int appID, int parseID) {
    const int recordsCount = octosql_get_records_count(appID, parseID);

    // dict is a borrowed reference.
    PyObject* dict = PyModule_GetDict(module);
    if (dict == nullptr) {
    PyErr_Print();
    std::cout << "Fails to get the dictionary.\n";
    return NULL;
    }

    // Builds the name of a callable class
    PyObject* python_class = PyDict_GetItemString(dict, "RecordSet");
    if (python_class == nullptr) {
    PyErr_Print();
    std::cout << "Fails to get the Python class.\n";
    return NULL;
    }

    PyObject* object;
    // Creates an instance of the class
    if (PyCallable_Check(python_class)) {
    object = PyObject_CallObject(python_class, nullptr);
    } else {
    std::cout << "Cannot instantiate the Python class" << std::endl;
    return NULL;
    }

    RecordObjectCapsule* cap = (RecordObjectCapsule*) malloc(sizeof(RecordObjectCapsule));
    cap->appID = appID;
    cap->parseID = parseID;
    cap->recordID = 0;

    PyObject* pycap = PyCapsule_New((void *)cap, "NATIVE_RECORD", RecordObjectCapsule_destroy);
    ((RecordObject*) object)->pycap = pycap;

    return object;
}

static PyObject* run(PyObject *self, PyObject *args) {
    int appID, parseID;

    if (!PyArg_ParseTuple(args, "ll", &appID, &parseID)) {
        return NULL;
    }

    octosql_run(appID, parseID);

    return get_query_results_obj(appID, parseID);
}

static PyMappingMethods RecordTypeMappingMethods = {
    .mp_subscript = get_query_record_val,
};

static PyTypeObject RecordType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "octosql_py_native.Record",
    .tp_doc = "Record entry",
    .tp_basicsize = sizeof(RecordObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_as_mapping = &RecordTypeMappingMethods,
    .tp_getattro = get_record_obj_attr,
};

static PyMappingMethods RecordSetTypeMappingMethods = {
    .mp_subscript = get_query_record_set_val,
};

static PyTypeObject RecordSetType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "octosql_py_native.RecordSet",
    .tp_doc = "Records set",
    .tp_basicsize = sizeof(RecordSetObject),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT,
    .tp_new = PyType_GenericNew,
    .tp_as_mapping = &RecordSetTypeMappingMethods,
    .tp_getattro = get_record_set_obj_attr,
};

PyMethodDef method_table[] = {
    {"init", (PyCFunction) init, METH_VARARGS, "Method docstring"},
    {"new_instance", (PyCFunction) new_instance, METH_VARARGS, "Method docstring"},
    {"create_new_parse", (PyCFunction) create_new_parse, METH_VARARGS, "Method docstring"},
    {"parse", (PyCFunction) parse, METH_VARARGS, "Method docstring"},
    {"plan", (PyCFunction) plan, METH_VARARGS, "Method docstring"},
    {"run", (PyCFunction) run, METH_VARARGS, "Method docstring"},
    {NULL, NULL, 0, NULL} // Sentinel value ending the table
};

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
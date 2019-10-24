#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <iostream>
#include "./libgooctosql.h"
#include "./helper.h"

#if PY_MAJOR_VERSION >= 3
#define PY3K
#endif

#define DEBUG 0

#define dbg if (DEBUG)
#define dbgm if (DEBUG) std::cout << "\n" <<

static const char* PyStringLike_AsStringAndSize(PyObject* o, Py_ssize_t* size) {
    #ifdef PY3K
        if (PyUnicode_Check(o)) {
            return PyUnicode_AsUTF8AndSize(o, size);
        }
    #else
        if (PyUnicode_Check(o)) {
            PyObject* str = PyUnicode_AsUTF8String(o);
            char* ret = PyString_AsString(str);
            *size = strlen(ret);
            return ret;
        } else if (PyString_Check(o)) {
            char* buf;
            PyString_AsStringAndSize(o, &buf, size);
            return buf;
        }
    #endif

    *size = 0;
    return NULL;
}

static PyObject* PyStringLike_FromString(const char* input) {
    #ifdef PY3K
        return PyUnicode_FromString(input);
    #else
        return PyString_FromString(input);
    #endif
}

static bool PyStringLike_Check(PyObject* o) {
    #ifdef PY3K
        return PyUnicode_Check(o);
    #else
        return PyUnicode_Check(o) || PyString_Check(o);
    #endif
}

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
    dbgm "init: Init OctoSQL native interface";
    const int res = octosql_init();
    if (res != 0) {
        SET_GO_ERR(octosql_get_error());
        return NULL;
    }
    Py_RETURN_NONE;
}

static PyObject* new_instance(PyObject *self, PyObject *args) {
    dbgm "new_instance: Create new OctoSQL native instance";

    char* yamlConfiguration;
    if (!PyArg_ParseTuple(args, "s", &yamlConfiguration)) {
        return NULL;
    }
    const int id = octosql_new_instance(strToGo(yamlConfiguration));

    dbgm "new_instance: Return generated ID";
    return PyLong_FromLong(id);
}

static PyObject* get_query_record_dict(int appID, int parseID, int recordID) {
    dbgm "get_query_record_dict: Create dict";

    PyObject *record = PyDict_New();
    const int fieldsCount = octosql_get_record_fields_count(appID, parseID, recordID);
    for (int i=0; i<fieldsCount; ++i) {
        PyObject* fieldVal = get_query_record_field_value(appID, parseID, recordID, i);
        if (fieldVal != NULL) {
            char* fieldName = octosql_get_record_field_name(appID, parseID, recordID, i);
            PyDict_SetItemString(record, fieldName, fieldVal);
        }
    }

    dbgm "get_query_record_dict: Return";
    return record;
}

static PyObject* create_record_native_wrapper(int appID, int parseID, int recordID) {
    dbgm "create_record_native_wrapper: Get module as dictionary";

    // dict is a borrowed reference.
    PyObject* dict = PyModule_GetDict(module);
    if (dict == NULL) {
        PyErr_Print();
        std::cout << "Fails to get the dictionary.\n";
        return NULL;
    }

    dbgm "create_fields_list: Get callable type constructor";
    // Builds the name of a callable class
    PyObject* python_class = PyDict_GetItemString(dict, "Record");
        if (python_class == NULL) {
        PyErr_Print();
        std::cout << "Fails to get the Python class.\n";
        return NULL;
    }

    dbgm "create_fields_list: Create new instance of object";
    PyObject* object;
    // Creates an instance of the class
    if (PyCallable_Check(python_class)) {
        object = PyObject_CallObject(python_class, NULL);
    } else {
        std::cout << "Cannot instantiate the Python class" << std::endl;
        return NULL;
    }

    dbgm "create_fields_list: Fill record capsule";
    RecordObjectCapsule* cap = (RecordObjectCapsule*) malloc(sizeof(RecordObjectCapsule));
    cap->appID = appID;
    cap->parseID = parseID;
    cap->recordID = recordID;

    PyObject* pycap = PyCapsule_New((void *)cap, "NATIVE_RECORD", RecordObjectCapsule_destroy);
    ((RecordObject*) object)->pycap = pycap;

    dbgm "create_fields_list: Return newly created object";
    return object;
}

static PyObject* create_fields_list(int appID, int parseID, int recordID) {
    dbgm "create_fields_list: Generate fields";
    const int fieldsCount = octosql_get_record_fields_count(appID, parseID, recordID);
    PyObject *fieldsList = PyList_New(fieldsCount);
    for (int i=0; i<fieldsCount; ++i) {
        const char* fieldName = octosql_get_record_field_name(appID, parseID, recordID, i);
        PyObject* object = PyStringLike_FromString(fieldName);
        PyList_SetItem(fieldsList, i, object);
    }
    dbgm "create_fields_list: Return fields";
    return fieldsList;
}

static PyObject* get_record_set_obj_attr(PyObject *self, PyObject *args) {
    dbgm "RecordSet: Get attributes";
    dbgm "RecordSet args type is [" << std::string(Py_TYPE(args)->tp_name) << "]\n";

    if(PyStringLike_Check(args)) {
        Py_ssize_t size;
        const char *ptr = PyStringLike_AsStringAndSize(args, &size);

        PyObject* pycap = ((RecordObject*) self)->pycap;

        RecordObjectCapsule *cap;
        cap = (RecordObjectCapsule*) PyCapsule_GetPointer(pycap, "NATIVE_RECORD");
        const int appID = cap->appID;
        const int parseID = cap->parseID;
        if (strcmp(ptr, "count") == 0) {
            return PyLong_FromLong(octosql_get_records_count(appID, parseID));
        } else if (strcmp(ptr, "records") == 0) {
            dbgm "RecordSet.records: Generating records";
            const int recordsCount = octosql_get_records_count(appID, parseID);

            PyObject *recordsList = PyList_New(recordsCount);
            for(int i=0;i<recordsCount;++i) {
                PyList_SetItem(recordsList, i, create_record_native_wrapper(appID, parseID, i));
            }
            return recordsList;
        } else if (strcmp(ptr, "fields") == 0) {
            dbgm "RecordSet.fields: Generating fields list";
            const int recordsCount = octosql_get_records_count(appID, parseID);
            if (recordsCount > 0) {
                return create_fields_list(appID, parseID, 0);
            }
            return PyList_New(0);
        } else if (strcmp(ptr, "values") == 0) {
            dbgm "RecordSet.values: Generating dictionary";
            const int recordsCount = octosql_get_records_count(appID, parseID);
            PyObject *recordsList = PyList_New(recordsCount);
            for (int i=0; i<recordsCount; ++i) {
                PyList_SetItem(recordsList, i, get_query_record_dict(appID, parseID, i));
            }
            return recordsList;
        }

        dbgm "RecordSet.values: Unknown field";
        Py_RETURN_NONE;
    }

    dbgm "RecordSet.values: NULL return";
    return NULL;
}

static PyObject* get_record_obj_attr(PyObject *self, PyObject *args) {
    dbgm "Record: Get attributes";
    if(PyStringLike_Check(args)) {
        Py_ssize_t size;
        const char *ptr = PyStringLike_AsStringAndSize(args, &size);

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
    dbgm "get_query_record_field_value: Get field value " << appID << ", " << parseID << ", " << recordID << ", " << fieldID;
    PyObject* fieldVal = NULL;
    const int fieldType = octosql_get_record_field_type(appID, parseID, recordID, fieldID);
    switch (fieldType) {
        case 0:
            fieldVal = PyLong_FromLong(octosql_get_record_field_as_int(appID, parseID, recordID, fieldID));
            break;
        case 3:
            fieldVal = PyStringLike_FromString(octosql_get_record_field_as_string(appID, parseID, recordID, fieldID));
            break;
        case 6:
            fieldVal = PyFloat_FromDouble(octosql_get_record_field_as_float(appID, parseID, recordID, fieldID));
            break;
        default:
            dbgm "get_query_record_field_value: not recognized type = " << fieldType;
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
    } else if(PyStringLike_Check(args)) {
        Py_ssize_t size;
        const char *ptr = PyStringLike_AsStringAndSize(args, &size);
        GoString goStr = {
            .p = ptr,
            .n = size,
        };

        fieldID = octosql_get_record_field_id(appID, parseID, recordID, goStr);
    }

    return get_query_record_field_value(appID, parseID, recordID, fieldID);
}

static PyObject* get_query_results_obj(int appID, int parseID) {
    // dict is a borrowed reference.
    PyObject* dict = PyModule_GetDict(module);
    if (dict == NULL) {
        PyErr_Print();
        std::cout << "Fails to get the dictionary.\n";
        return NULL;
    }

    // Builds the name of a callable class
    PyObject* python_class = PyDict_GetItemString(dict, "RecordSet");
    if (python_class == NULL) {
        PyErr_Print();
        std::cout << "Fails to get the Python class.\n";
        return NULL;
    }

    PyObject* object;
    // Creates an instance of the class
    if (PyCallable_Check(python_class)) {
        object = PyObject_CallObject(python_class, NULL);
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
    0, /* mp_length */
    get_query_record_val,  /* mp_subscript */
    0,  /* mp_ass_subscript */
};

static PyTypeObject RecordType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "octosql_py_native.Record", /* tp_name */
    sizeof(RecordObject),       /* tp_basicsize */
    0,                          /* tp_itemsize */
    0,                          /* tp_dealloc */
    0,                          /* tp_print */
    0,                          /* tp_getattr */
    0,                          /* tp_setattr */
    0,                          /* tp_reserved */
    0,                          /* tp_repr */
    0,                          /* tp_as_number */
    0,                          /* tp_as_sequence */
    &RecordTypeMappingMethods,  /* tp_as_mapping */
    0,                          /* tp_hash  */
    0,                          /* tp_call */
    0,                          /* tp_str */
    get_record_obj_attr,        /* tp_getattro */
    0,                          /* tp_setattro */
    0,                          /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,         /* tp_flags */
    "Record entry object",      /* tp_doc */
};

static PyMappingMethods RecordSetTypeMappingMethods = {
    0, /* mp_length */
    get_query_record_set_val,  /* mp_subscript */
    0,  /* mp_ass_subscript */
};

static PyTypeObject RecordSetType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "octosql_py_native.RecordSet", /* tp_name */
    sizeof(RecordSetObject),   /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    &RecordSetTypeMappingMethods, /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    get_record_set_obj_attr,   /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT,        /* tp_flags */
    "Record set object",       /* tp_doc */
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
    if (PyType_Ready(&RecordType) < 0)
        return;
    if (PyType_Ready(&RecordSetType) < 0)
            return;

    if (module == NULL)
        return;

    if (PyModule_AddObject(module, "Record", (PyObject *) &RecordType) < 0) {
        return;
    }
    if (PyModule_AddObject(module, "RecordSet", (PyObject *) &RecordSetType) < 0) {
        return;
    }

    return;
}
#endif
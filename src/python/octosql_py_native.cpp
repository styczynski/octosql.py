#define DEBUG 1
#define PY_SSIZE_T_CLEAN

#include <Python.h>
#include <iostream>
#include <functional>
#include "debug.h"
#include "libgooctosql.h"
#include "helper.h"
#include "structmember.h"
#include "../custom_storage/custom_storage.hpp"

static PyTypeObject* RecordSetType_get();
static PyTypeObject* RecordType_get();

PyObject* module = NULL;

typedef struct {
    int appID;
    int parseID;
    int recordID;
} RecordObjectCapsule;

void RecordObjectCapsule_destroy(PyObject *pycap) {
    dbgm "RecordObjectCapsule_destroy - started";
    RecordObjectCapsule *cap;
    cap = (RecordObjectCapsule*) PyCapsule_GetPointer(pycap, "NATIVE_RECORD");
    if (cap == NULL) {
         return;
    }

    dbgm "RecordObjectCapsule_destroy - free";
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
        dbgm "Init function errored";
        SET_GO_ERR(octosql_get_error());
        return NULL;
    }
    dbgm "Init completed";
    Py_RETURN_NONE;
}

static std::vector<std::function<NativeSourceRecord()>> source_handlers;
static int source_handler_free_id = 0;

NativeSourceRecord octosql_register_native_source_indirect_impl(int ptr) {
    return source_handlers[ptr]();
}

static PyObject* create_native_source(PyObject *self, PyObject *args) {

    dbgm "Deserialize factory";
    PyObject* recordFactory;
    if (!PyArg_ParseTuple(args, "O", &recordFactory)) {
        return NULL;
    }
    Py_INCREF(args);
    Py_INCREF(recordFactory);

    int new_source_id = 0;
    std::function<NativeSourceRecord()> fun = [=](){
        NativeSourceRecord record;

        dbgm "Call custom record method";
        //PyObject* pyRecord = PyObject_CallFunction(recordFactory, "s", val.c_str());
        PyObject* pyRecord = PyObject_CallMethod(recordFactory, "_next_record_", "i", new_source_id);
        //Py_XINCREF(pyRecord);
        dbgm "Call end -> " << ((int)(size_t)pyRecord);

        if (Py_None == pyRecord) {
            // End of stream
            Py_XDECREF(recordFactory);
            Py_XDECREF(args);
            return record;
        } else if (PyDict_Check(pyRecord)) {
            PyObject *key, *value;
            Py_ssize_t pos = 0;

            while (PyDict_Next(pyRecord, &pos, &key, &value)) {
                NativeSourceValue field;
                field.name = PyStringLike_AsCppString(key);
                field.value = 0;
                field.type = 0;
                if (PyLong_Check(value)) {
                    field.value = (void*) PyLong_AsLong(value);
                    field.type = 0;
                    record.fields.push_back(field);
                } else if (PyStringLike_Check(value)) {
                    std::string* fstr = (std::string*) malloc(sizeof(std::string));
                    *fstr = PyStringLike_AsCppString(value);
                    field.value = fstr;
                    field.type = 3;
                    record.fields.push_back(field);
                }
            }
        }

        Py_XDECREF(pyRecord);
        dbgm "Return record";
        return record;
    };

    source_handlers.push_back(fun);
    const int fun_id = source_handler_free_id++;

    NativeSourceRecord empty_record;
    NativeSource new_source = {
        fun_id,
        octosql_register_native_source_indirect_impl,
        empty_record,
    };

    new_source_id = octosql_register_native_source(new_source);
    return PyLong_FromLong(new_source_id);
}

static PyObject* new_instance(PyObject *self, PyObject *args) {
    dbgm "new_instance: Create new OctoSQL native instance";

    char* yamlConfiguration;
    if (!PyArg_ParseTuple(args, "s", &yamlConfiguration)) {
        return NULL;
    }

    dbgm "new_instance: Call native method";
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

    dbgm "create_fields_list: Fill record capsule";
    RecordObjectCapsule* cap = (RecordObjectCapsule*) malloc(sizeof(RecordObjectCapsule));
    cap->appID = appID;
    cap->parseID = parseID;
    cap->recordID = recordID;

    dbgm "get_query_results_obj - capsule pack the val";
    PyObject* pycap = PyCapsule_New((void *)cap, "NATIVE_RECORD", RecordObjectCapsule_destroy);

    dbgm "get_query_results_obj - capsule set";
    PyObject* argList = Py_BuildValue("(O)", pycap);
    PyObject* object = PyObject_CallObject((PyObject *) RecordType_get(), argList);
    Py_DECREF(argList);

    dbgm "create_fields_list: Return newly created object";
    return (PyObject*) object;
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
    dbgm "Create new parse";
    

    PyObject* o = PyLong_FromLong(octosql_create_new_parse());

    dbgm "Create new parse - end";
    
    return o;
}

static PyObject* parse(PyObject *self, PyObject *args) {
    long int appID, parseID;
    char* input;

    if (!PyArg_ParseTuple(args, "lls", &appID, &parseID, &input)) {
        return NULL;
    }

    octosql_parse(appID, parseID, strToGo(input));
    dbgm "Parse done!";
    
    Py_RETURN_NONE;
}

static PyObject* plan(PyObject *self, PyObject *args) {
    long int appID, parseID;

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
    dbgm "get_query_results_obj - started";

    dbgm "get_query_results_obj - capsule";
    
    RecordObjectCapsule* cap = (RecordObjectCapsule*) malloc(sizeof(RecordObjectCapsule));
    cap->appID = appID;
    cap->parseID = parseID;
    cap->recordID = 0;

    dbgm "get_query_results_obj - capsule pack it";
    
    PyObject* pycap = PyCapsule_New((void *)cap, "NATIVE_RECORD", RecordObjectCapsule_destroy);

    dbgm "get_query_results_obj - capsule set";
    PyObject* argList = Py_BuildValue("(O)", pycap);
    PyObject* object = PyObject_CallObject((PyObject *) RecordSetType_get(), argList);
    Py_DECREF(argList);

    dbgm "get_query_results_obj - return";
    
    return (PyObject*) object;
}

static PyObject* run(PyObject *self, PyObject *args) {
    long int appID, parseID;
    dbgm "run() - started";
    

    if (!PyArg_ParseTuple(args, "ll", &appID, &parseID)) {
        return NULL;
    }

    octosql_run(appID, parseID);

    dbgm "run() - end now get query results";
    
    return get_query_results_obj(appID, parseID);
}

static int RecordSetType_init(RecordObject *self, PyObject *args, PyObject *kwargs) {

    PyObject* pycap;
    if (!PyArg_ParseTuple(args, "O", &pycap)) {
        return 1;
    }

    self->pycap = pycap;

    return 0;
}

static int RecordType_init(RecordObject *self, PyObject *args, PyObject *kwargs) {

    PyObject* pycap;
    if (!PyArg_ParseTuple(args, "O", &pycap)) {
        return 1;
    }

    self->pycap = pycap;

    return 0;
}

static PyMappingMethods RecordTypeMappingMethods = {
    0, /* mp_length */
    get_query_record_val,  /* mp_subscript */
    0,  /* mp_ass_subscript */
};

static PyMemberDef RecordTypeMembers[] = {
    {"pycap", T_OBJECT_EX, offsetof(RecordObject, pycap), 0, "Native record identifier"},
    {NULL}
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
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    0,                              /* tp_methods */
    RecordTypeMembers,              /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc) RecordType_init,     /* tp_init */
    0,                              /* tp_alloc */
    PyType_GenericNew,                      /* tp_new */
};


static PyTypeObject* RecordType_get() {
    return &RecordType;
}

static PyMappingMethods RecordSetTypeMappingMethods = {
    0, /* mp_length */
    get_query_record_set_val,  /* mp_subscript */
    0,  /* mp_ass_subscript */
};

static PyMemberDef RecordSetTypeMembers[] = {
    {"pycap", T_OBJECT_EX, offsetof(RecordSetObject, pycap), 0, "Native record set identifier"},
    {NULL}
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
    0,                              /* tp_traverse */
    0,                              /* tp_clear */
    0,                              /* tp_richcompare */
    0,                              /* tp_weaklistoffset */
    0,                              /* tp_iter */
    0,                              /* tp_iternext */
    0,                              /* tp_methods */
    RecordSetTypeMembers,           /* tp_members */
    0,                              /* tp_getset */
    0,                              /* tp_base */
    0,                              /* tp_dict */
    0,                              /* tp_descr_get */
    0,                              /* tp_descr_set */
    0,                              /* tp_dictoffset */
    (initproc) RecordSetType_init,  /* tp_init */
    0,                              /* tp_alloc */
    PyType_GenericNew,                      /* tp_new */
};

static PyTypeObject* RecordSetType_get() {
    return &RecordSetType;
}

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
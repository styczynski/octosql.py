#include "../inc/core.h"
#include "../inc/record_obj.h"
#include "../inc/structmember.h"

int RecordType_init(RecordObject *self, PyObject *args, PyObject *kwargs) {

    PyObject* pycap;
    if (!PyArg_ParseTuple(args, "O", &pycap)) {
        return 1;
    }

    self->pycap = pycap;

    return 0;
}


PyObject* get_record_obj_attr(PyObject *self, PyObject *args) {
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


PyObject* get_query_record_dict(int appID, int parseID, int recordID) {
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

PyMappingMethods RecordTypeMappingMethods = {
    0, /* mp_length */
    get_query_record_val,  /* mp_subscript */
    0,  /* mp_ass_subscript */
};

PyMemberDef RecordTypeMembers[] = {
    {"pycap", T_OBJECT_EX, offsetof(RecordObject, pycap), 0, "Native record identifier"},
    {NULL}
};

PyTypeObject RecordType = {
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


PyTypeObject* RecordType_get() {
    return &RecordType;
}

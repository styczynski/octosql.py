#include "../inc/core.h"
#include "../inc/record_set_obj.h"
#include "../inc/structmember.h"

int RecordSetType_init(RecordObject *self, PyObject *args, PyObject *kwargs) {

    PyObject* pycap;
    if (!PyArg_ParseTuple(args, "O", &pycap)) {
        return 1;
    }

    self->pycap = pycap;

    return 0;
}


PyMappingMethods RecordSetTypeMappingMethods = {
    0, /* mp_length */
    get_query_record_set_val,  /* mp_subscript */
    0,  /* mp_ass_subscript */
};

PyMemberDef RecordSetTypeMembers[] = {
    {"pycap", T_OBJECT_EX, offsetof(RecordSetObject, pycap), 0, "Native record set identifier"},
    {NULL}
};

PyTypeObject RecordSetType = {
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

PyTypeObject* RecordSetType_get() {
    return &RecordSetType;
}


PyObject* get_record_set_obj_attr(PyObject *self, PyObject *args) {
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

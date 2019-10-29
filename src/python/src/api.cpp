#include "../inc/core.h"
#include "../inc/api.h"

PyObject* init(PyObject *self, PyObject *args) {
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


PyObject* new_instance(PyObject *self, PyObject *args) {
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

PyObject* create_record_native_wrapper(int appID, int parseID, int recordID) {
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

PyObject* create_fields_list(int appID, int parseID, int recordID) {
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

PyObject* create_new_parse(PyObject *self, PyObject *args) {
    dbgm "Create new parse";


    PyObject* o = PyLong_FromLong(octosql_create_new_parse());

    dbgm "Create new parse - end";

    return o;
}

PyObject* parse(PyObject *self, PyObject *args) {
    long int appID, parseID;
    char* input;

    if (!PyArg_ParseTuple(args, "lls", &appID, &parseID, &input)) {
        return NULL;
    }

    octosql_parse(appID, parseID, strToGo(input));
    dbgm "Parse done!";

    Py_RETURN_NONE;
}

PyObject* plan(PyObject *self, PyObject *args) {
    long int appID, parseID;

    if (!PyArg_ParseTuple(args, "ll", &appID, &parseID)) {
        return NULL;
    }

    octosql_plan(appID, parseID);
    Py_RETURN_NONE;
}

PyObject* get_query_record_field_value(int appID, int parseID, int recordID, int fieldID) {
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


PyObject* get_query_record_set_val(PyObject *self, PyObject *args) {
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

PyObject* get_query_record_val(PyObject *self, PyObject *args) {
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

PyObject* get_query_results_obj(int appID, int parseID) {
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

PyObject* run(PyObject *self, PyObject *args) {
    long int appID, parseID;
    dbgm "run() - started";


    if (!PyArg_ParseTuple(args, "ll", &appID, &parseID)) {
        return NULL;
    }

    octosql_run(appID, parseID);

    dbgm "run() - end now get query results";

    return get_query_results_obj(appID, parseID);
}
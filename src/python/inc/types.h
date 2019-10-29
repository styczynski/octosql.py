#ifndef OCTSQLN_PY_TYPES
#define OCTSQLN_PY_TYPES

typedef struct {
    int appID;
    int parseID;
    int recordID;
} RecordObjectCapsule;

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

#endif // OCTSQLN_PY_TYPES
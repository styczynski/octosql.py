#include <string>

#define SET_GO_ERR(VAL) if (true) { \
        char ___goErrBuf___ [500]; \
        const GoString ___goErrStr___ = ( VAL ); \
        PyErr_SetString(PyExc_Exception, strncpy(___goErrBuf___, ___goErrStr___.p, ___goErrStr___.n)); \
    }

GoString strToGo(char* str) {
    GoString goStr = {
        .p = str,
        .n = (ptrdiff_t) strlen(str),
    };
    return goStr;
}

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


static std::string PyStringLike_AsCppString(PyObject* o) {
    Py_ssize_t size;
    return std::string(PyStringLike_AsStringAndSize(o, &size));
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
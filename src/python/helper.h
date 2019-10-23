#define SET_GO_ERR(VAL) if (true) { \
        char ___goErrBuf___ [500]; \
        const GoString ___goErrStr___ = ( VAL ); \
        PyErr_SetString(PyExc_Exception, strncpy(___goErrBuf___, ___goErrStr___.p, ___goErrStr___.n)); \
    }

GoString strToGo(char* str) {
    GoString goStr = {
        .p = str,
        .n = strlen(str),
    };
    return goStr;
}
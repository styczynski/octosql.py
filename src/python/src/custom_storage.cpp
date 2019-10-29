#include "../inc/core.h"
#include "../inc/custom_storage.h"
#include "../inc/parse_python_record.h"

PyObject* create_native_source(PyObject *self, PyObject *args) {

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

        if(!parse_field(&record, pyRecord)) {
            // End of stream
            Py_XDECREF(recordFactory);
            Py_XDECREF(args);
            dbgm "end_of_stream";
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

NativeSourceRecord octosql_register_native_source_indirect_impl(int ptr) {
    return source_handlers[ptr]();
}


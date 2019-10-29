#include "../inc/core.h"
#include "../inc/parse_python_record.h"
#include "../inc/helper.h"

bool parse_field(NativeSourceRecord* record, PyObject* obj);

void parse_field_dict(NativeSourceRecord* record, PyObject* key, PyObject* value) {
    NativeSourceValue field;
    field.name = PyStringLike_AsCppString(key);
    field.value = 0;
    field.type = 0;
    if (PyLong_Check(value)) {
        field.value = (void*) PyLong_AsLong(value);
        field.type = 0;
        record->fields.push_back(field);
        dbgm "add_long_field";
    } else if (PyInt_Check(value)) {
         field.value = (void*) PyInt_AsLong(value);
         field.type = 0;
         record->fields.push_back(field);
    } else if (PyStringLike_Check(value)) {
        std::string* fstr = (std::string*) malloc(sizeof(std::string));
        *fstr = PyStringLike_AsCppString(value);
        field.value = fstr;
        field.type = 3;
        record->fields.push_back(field);
        dbgm "add_string_field";
    } else if (PyTuple_Check(value)) {
        const Py_ssize_t len = PyTuple_Size(value);
        if (len == 0) {
            return;
        } else if (len == 1) {
            NativeSourceRecord buf;
            parse_field(&buf, PyTuple_GetItem(value, 0));
            for (int i=0;i<buf.fields.size();++i) {
                record->fields.push_back(buf.fields[i]);
            }
        } else {
            dbgm "Not supported tuple size = " << len;
        }
    } else {
        dbgm "add_other_field: " << std::string(Py_TYPE(value)->tp_name);
    }
}

bool parse_field(NativeSourceRecord* record, PyObject* obj) {
    if (Py_None == obj) {
        // End of stream
        return false;
    } else if (PyDict_Check(obj)) {
        PyObject *key, *value;
        Py_ssize_t pos = 0;

        dbgm "add_fields...";
        while (PyDict_Next(obj, &pos, &key, &value)) {
            parse_field_dict(record, key, value);
        }
        dbgm "add_fields - end";
    }

    return true;
}

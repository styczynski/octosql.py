#ifndef OCTSQLN_PY_PARSE_PYTHON_RECORD
#define OCTSQLN_PY_PARSE_PYTHON_RECORD

PyObject* get_query_record_field_value(int appID, int parseID, int recordID, int fieldID);
bool parse_field(NativeSourceRecord* record, PyObject* obj);

#endif // OCTSQLN_PY_PARSE_PYTHON_RECORD
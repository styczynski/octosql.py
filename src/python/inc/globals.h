#ifndef OCTSQLN_PY_GLOBALS
#define OCTSQLN_PY_GLOBALS

#include "./core.h"
#include "../../custom_storage/custom_storage.hpp"

static PyTypeObject* RecordSetType_get();
static PyTypeObject* RecordType_get();

static std::vector<std::function<NativeSourceRecord()>> source_handlers;
static int source_handler_free_id = 0;

static PyObject* module = NULL;

#endif // OCTSQLN_PY_GLOBALS
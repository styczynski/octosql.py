#include <map>
#include <string>
#include <vector>
#include <functional>
#include "custom_storage.h"

typedef struct {
    void* value;
    int type;
    std::string name;
} NativeSourceValue;

typedef struct {
    std::vector<NativeSourceValue> fields;
} NativeSourceRecord;

typedef struct {
    //std::function<NativeSourceRecord()>
    int readNextRecord;
    NativeSourceRecord (*readNextRecordIndirect)(int);
    NativeSourceRecord recordBuf;
} NativeSource;

typedef int NativeSourceID;

NativeSourceID octosql_register_native_source(NativeSource source);
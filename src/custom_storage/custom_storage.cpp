#include "custom_storage.hpp"
#include <iostream>

#define dgbm_stor std::cout << "\n" <<

static std::map<int, NativeSource> nativeSources;
static NativeSourceID nativeSourcesFreeID = 0;

NativeSourceID octosql_register_native_source(NativeSource source) {
    nativeSources[nativeSourcesFreeID] = source;
    return nativeSourcesFreeID++;
}

void octosql_native_source_read_next_record(NativeSourceID id) {
    dgbm_stor "octosql_native_source_read_next_record";
    nativeSources[id].recordBuf = nativeSources[id].readNextRecordIndirect(nativeSources[id].readNextRecord);
}

int octosql_native_source_get_record_fields_count(NativeSourceID id) {
    dgbm_stor "octosql_native_source_get_record_fields_count = " << nativeSources[id].recordBuf.fields.size();
    return nativeSources[id].recordBuf.fields.size();
}

int octosql_native_source_get_record_field_type(NativeSourceID id, int fieldID) {
    dgbm_stor "octosql_native_source_get_record_field_type = " << nativeSources[id].recordBuf.fields[fieldID].type;
    return nativeSources[id].recordBuf.fields[fieldID].type;
}

const char* octosql_native_source_get_record_field_as_string(NativeSourceID id, int fieldID) {
    dgbm_stor "octosql_native_source_get_record_field_as_string";
    return ((std::string*) (nativeSources[id].recordBuf.fields[fieldID].value))->c_str();
}

int octosql_native_source_get_record_field_as_int(NativeSourceID id, int fieldID) {
    dgbm_stor "octosql_native_source_get_record_field_as_int";
    return ((int)(size_t)(nativeSources[id].recordBuf.fields[fieldID].value));
}

const char* octosql_native_source_get_record_field_name(NativeSourceID id, int fieldID) {
    dgbm_stor "octosql_native_source_get_record_field_name";
    return nativeSources[id].recordBuf.fields[fieldID].name.c_str();
}
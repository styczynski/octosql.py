#ifdef __cplusplus
extern "C" {
#endif

typedef int NativeSourceID;

void octosql_native_source_read_next_record(NativeSourceID id);

int octosql_native_source_get_record_fields_count(NativeSourceID id);

int octosql_native_source_get_record_field_type(NativeSourceID id, int fieldID);

const char* octosql_native_source_get_record_field_name(NativeSourceID id, int fieldID);

const char* octosql_native_source_get_record_field_as_string(NativeSourceID id, int fieldID);

int octosql_native_source_get_record_field_as_int(NativeSourceID id, int fieldID);


#ifdef __cplusplus
}
#endif
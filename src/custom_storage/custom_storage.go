package custom_storage
//  go build -o "./out/out" -buildmode=c-archive ./src/lib.go

// #include "./custom_storage.h"
// #cgo CFLAGS: -I.
// #cgo LDFLAGS: -L. -lcustom_storage_octosql
import "C"

import (
	"context"
	"github.com/cube2222/octosql"
	"github.com/cube2222/octosql/config"
	"github.com/cube2222/octosql/execution"
	"github.com/cube2222/octosql/physical"
	"github.com/cube2222/octosql/physical/metadata"
	"github.com/pkg/errors"
	"strconv"
)

type DataSource struct {
	alias       string
	nativeID    int32
}

var availableFilters = map[physical.FieldType]map[physical.Relation]struct{}{
	physical.Primary:   make(map[physical.Relation]struct{}),
	physical.Secondary: make(map[physical.Relation]struct{}),
}

func NewDataSourceBuilderFactory() physical.DataSourceBuilderFactory {
	return physical.NewDataSourceBuilderFactory(
		func(ctx context.Context, matCtx *physical.MaterializationContext, dbConfig map[string]interface{}, filter physical.Formula, alias string) (execution.Node, error) {
			var id int32
			strID, err := config.GetString(dbConfig, "id")
			if err != nil {
				newID, retryErr := config.GetInt(dbConfig, "id")
				if retryErr != nil {
					return nil, errors.Wrap(err, "couldn't get custom stoarge id")
				} else {
					id = int32(newID)
				}
			} else {
				newID, err := strconv.Atoi(strID)
				if err != nil {
					return nil, errors.Wrap(err, "couldn't get custom stoarge id - not a number")
				}
				id = int32(newID)
			}

			return &DataSource{
				alias:       alias,
				nativeID:    id,
			}, nil
		},
		nil,
		availableFilters,
		metadata.BoundedFitsInLocalStorage,
	)
}

// NewDataSourceBuilderFactoryFromConfig creates a data source builder factory using the configuration.
func NewDataSourceBuilderFactoryFromConfig(dbConfig map[string]interface{}) (physical.DataSourceBuilderFactory, error) {
	return NewDataSourceBuilderFactory(), nil
}

func (ds *DataSource) Get(variables octosql.Variables) (execution.RecordStream, error) {
	return &RecordStream{
		isDone:                        false,
		alias:                         ds.alias,
		nativeID:                      ds.nativeID,
	}, nil
}

type RecordStream struct {
	isDone                        bool
	alias                         string
	nativeID                      int32
}

func (rs *RecordStream) Close() error {
	return nil
}

func (rs *RecordStream) Next() (*execution.Record, error) {
	if rs.isDone {
		return nil, execution.ErrEndOfStream
	}

	var fields = []octosql.VariableName{}
	var values = map[octosql.VariableName]octosql.Value{}

	C.octosql_native_source_read_next_record(C.int(rs.nativeID))
	fieldsCount := int(C.octosql_native_source_get_record_fields_count(C.int(rs.nativeID)))
	for fieldID := 0; fieldID < fieldsCount; fieldID++ {
		typeID := int(C.octosql_native_source_get_record_field_type(C.int(rs.nativeID), C.int(fieldID)))
		switch typeID {
			case 0:
				val := int(C.octosql_native_source_get_record_field_as_int(C.int(rs.nativeID), C.int(fieldID)))
				name := octosql.VariableName(C.GoString(C.octosql_native_source_get_record_field_name(C.int(rs.nativeID), C.int(fieldID))))
				fields = append(fields, name)
				values[name] = octosql.NormalizeType(val)
				break;
			case 3:
				val := C.GoString(C.octosql_native_source_get_record_field_as_string(C.int(rs.nativeID), C.int(fieldID)))
				name := octosql.VariableName(C.GoString(C.octosql_native_source_get_record_field_name(C.int(rs.nativeID), C.int(fieldID))))
				fields = append(fields, name)
				values[name] = octosql.NormalizeType(val)
				break;
		}
	}

	if fieldsCount == 0 {
		rs.isDone = true
		return nil, execution.ErrEndOfStream
	}
	return execution.NewRecord(fields, values), nil
}
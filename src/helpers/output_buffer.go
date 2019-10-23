package helpers

import (
	"github.com/cube2222/octosql"
	"github.com/cube2222/octosql/execution"
	"log"
	"strings"
	"sync"
)

type OctoSQLOutputBuffer struct {
	sync.RWMutex
	records []*execution.Record
	closed bool
}

func (out *OctoSQLOutputBuffer) GetRecordsCount() int32 {
	out.RLock()
	defer out.RUnlock()

	return int32(len(out.records))
}

func (out *OctoSQLOutputBuffer) GetRecordFieldID(id int32, fieldName string) int32 {
	out.RLock()
	defer out.RUnlock()

	r := out.records[id].AsVariables()
	i := 0
	for a, _ := range r {
		if strings.EqualFold(a.String(), fieldName) {
			return int32(i)
		}
		i++
	}
	println("default exit")
	return 0
}


func (out *OctoSQLOutputBuffer) GetRecordFieldsCount(id int32) int32 {
	out.RLock()
	defer out.RUnlock()

	return int32(len(out.records[id].Fields()))
}

func (out *OctoSQLOutputBuffer) getRecordFieldByID(id int32, fieldID int32) *execution.Field {
	return &out.records[id].Fields()[fieldID];
}

func (out *OctoSQLOutputBuffer) GetRecordFieldName(id int32, fieldID int32) string {
	out.RLock()
	defer out.RUnlock()

	return out.getRecordFieldByID(id, fieldID).Name.String()
}

func (out *OctoSQLOutputBuffer) GetRecordFieldType(id int32, fieldID int32) int32 {
	out.RLock()
	defer out.RUnlock()

	field := out.records[id].AsVariables()[out.getRecordFieldByID(id, fieldID).Name]
	switch field.(type) {
		case octosql.Int: return 0
		case octosql.Bool: return 1
		case octosql.Null: return 2
		case octosql.String: return 3
		case octosql.Tuple: return 4
		case octosql.Duration: return 5
		case octosql.Float: return 6
		case octosql.Time: return 7
		case octosql.Object: return 8
		default: return -1
	}
}

func (out *OctoSQLOutputBuffer) GetRecordFieldAsInt(id int32, fieldID int32) int32 {
	out.RLock()
	defer out.RUnlock()

	field := out.records[id].AsVariables()[out.getRecordFieldByID(id, fieldID).Name]
	switch field := field.(type) {
		case octosql.Int:
			return int32(field.AsInt())
	}
	log.Fatal("OctoSQLOutputBuffer: Invalid field type error")
	return 0
}

func (out *OctoSQLOutputBuffer) GetRecordFieldAsBool(id int32, fieldID int32) bool {
	out.RLock()
	defer out.RUnlock()

	field := out.records[id].AsVariables()[out.getRecordFieldByID(id, fieldID).Name]
	switch field := field.(type) {
	case octosql.Bool:
		return field.AsBool()
	}
	log.Fatal("OctoSQLOutputBuffer: Invalid field type error")
	return false
}

func (out *OctoSQLOutputBuffer) GetRecordFieldAsFloat(id int32, fieldID int32) float64 {
	out.RLock()
	defer out.RUnlock()

	field := out.records[id].AsVariables()[out.getRecordFieldByID(id, fieldID).Name]
	switch field := field.(type) {
	case octosql.Float:
		return field.AsFloat()
	}
	log.Fatal("OctoSQLOutputBuffer: Invalid field type error")
	return 0.0
}

func (out *OctoSQLOutputBuffer) GetRecordFieldAsString(id int32, fieldID int32) string {
	out.RLock()
	defer out.RUnlock()

	field := out.records[id].AsVariables()[out.getRecordFieldByID(id, fieldID).Name]
	switch field := field.(type) {
	case octosql.String:
		return field.AsString()
	}
	log.Fatal("OctoSQLOutputBuffer: Invalid field type error")
	return ""
}

func CreateOutputBuffer() (OctoSQLOutputBuffer) {
	return OctoSQLOutputBuffer{
		records: []*execution.Record{},
		closed: false,
	}
}

func (out *OctoSQLOutputBuffer) WriteRecord(record *execution.Record) error {
	out.Lock()
	defer out.Unlock()

	if out.closed {
		return nil
	}
	out.records = append(out.records, record)
	return nil
}

func (out *OctoSQLOutputBuffer) Close() error {
	out.Lock()
	defer out.Unlock()

	//out.records = []*execution.Record{}
	out.closed = true
	return nil
}

package main

import (
	"context"
	"github.com/cube2222/octosql/logical"
	"github.com/cube2222/octosql/storage/excel"
	"github.com/pkg/errors"
	"github.com/styczynski/octosql.py/src/custom_storage"
	"github.com/styczynski/octosql.py/src/helpers"
	"gopkg.in/yaml.v2"
	"log"
	"reflect"

	"github.com/cube2222/octosql/app"
	"github.com/cube2222/octosql/config"
	"github.com/cube2222/octosql/parser"
	"github.com/cube2222/octosql/parser/sqlparser"
	"github.com/cube2222/octosql/physical"
	"github.com/cube2222/octosql/storage/csv"
	"github.com/cube2222/octosql/storage/json"
	"github.com/cube2222/octosql/storage/mysql"
	"github.com/cube2222/octosql/storage/postgres"
	"github.com/cube2222/octosql/storage/redis"
)

import "C"

func main () {}

var appInstances *map[int32]*AppInstance = nil
var appInstancesFI int32 = 0
var appParseObjs *map[int32]*ParseInstance = nil
var appParseObjsFI int32 = 0
var appResultBuffers *map[int32]*helpers.OctoSQLOutputBuffer = nil
var appResultBuffersFI int32 = 0
var wasInited bool = false
var lastError string = ""

type ParseInstance struct {
	ast* sqlparser.SelectStatement
	plan* logical.Node
}

type AppInstance struct {
	context context.Context
	sources *physical.DataSourceRepository
	config *config.Config
	app *app.App
	bufferID int32
}

//export test
func test () {}

func deserializeConfiguration(yamlConfiguration string) (*config.Config, error) {
	var configuration config.Config

	err := yaml.Unmarshal([]byte(yamlConfiguration), &configuration)
	if err != nil {
		return nil, errors.Wrap(err, "couldn't decode yaml configuration")
	}

	return &configuration, nil
}

//export octosql_get_error
func octosql_get_error() (string) {
	return lastError
}

//export octosql_init
func octosql_init() (int32) {

	println("octosql_init()")
	if wasInited {
		print("wasInited oops!")
		lastError = "Octosql was already initialized"
		return 1
	}
	println("ok pass")

	appInstances = &map[int32]*AppInstance{}
	appInstancesFI = 0

	println("A")
	appParseObjs = &map[int32]*ParseInstance{}
	appParseObjsFI = 0

	println("B")
	appResultBuffers = &map[int32]*helpers.OctoSQLOutputBuffer{}
	appResultBuffersFI = 0

	println("C")
	wasInited = true

	println("Exit")
	return 0
}

//export octosql_new_instance
func octosql_new_instance(yamlConfiguration string) (int32) {
	var inst AppInstance = AppInstance{}
	var err error

	inst.config, err = deserializeConfiguration(yamlConfiguration)
	if err != nil {
		return -1
	}

	inst.sources, err = physical.CreateDataSourceRepositoryFromConfig(
		map[string]physical.Factory{
			"custom":   custom_storage.NewDataSourceBuilderFactoryFromConfig,
			"csv":      csv.NewDataSourceBuilderFactoryFromConfig,
			"json":     json.NewDataSourceBuilderFactoryFromConfig,
			"mysql":    mysql.NewDataSourceBuilderFactoryFromConfig,
			"postgres": postgres.NewDataSourceBuilderFactoryFromConfig,
			"redis":    redis.NewDataSourceBuilderFactoryFromConfig,
			"excel":    excel.NewDataSourceBuilderFactoryFromConfig,
		},
		inst.config,
	)

	if err != nil {
		return -1
	}

	var resultBuffer helpers.OctoSQLOutputBuffer = helpers.CreateOutputBuffer()

	(*appResultBuffers)[appResultBuffersFI] = &resultBuffer
	appResultBuffersFI++
	inst.bufferID = appResultBuffersFI-1

	inst.app = app.NewApp(inst.config, inst.sources, &resultBuffer)

	inst.context = context.Background()
	appInstancesFI++

	(*appInstances)[appInstancesFI-1] = &inst

	return appInstancesFI-1
}

//export octosql_create_new_parse
func octosql_create_new_parse() (int32) {
	var parseObj ParseInstance = ParseInstance{
		ast: nil,
	}

	(*appParseObjs)[appParseObjsFI] = &parseObj
	appParseObjsFI++

	return appParseObjsFI-1
}

//export octosql_parse
func octosql_parse(appID int32, parseID int32, input string) {
	parseObj := (*appParseObjs)[parseID]
	var err error
	var stmt sqlparser.Statement
	stmt, err = sqlparser.Parse(input)
	if err != nil {
		log.Fatal("couldn't parse query: ", err)
	}

	ast, ok := stmt.(sqlparser.SelectStatement)
	if !ok {
		log.Fatalf("invalid statement type, wanted sqlparser.SelectStatement got %v", reflect.TypeOf(stmt))
	}
	parseObj.ast = &ast
}


//export octosql_plan
func octosql_plan(appID int32, parseID int32) {
	parseObj := (*appParseObjs)[parseID]

	plan, err := parser.ParseNode(*parseObj.ast)
	if err != nil {
		log.Fatal("couldn't parse query: ", err)
	}

	parseObj.plan = &plan
}

//export octosql_delete_parse
func octosql_delete_parse(parseID int32) {
	delete(*appParseObjs, parseID)
}

//export octosql_delete_instance
func octosql_delete_instance(appID int32) {
	delete(*appInstances, appID)
}

//export octosql_run
func octosql_run(appID int32, parseID int32) (int32) {
	inst := (*appInstances)[appID]
	parseObj := (*appParseObjs)[parseID]

	err := inst.app.RunPlan(inst.context, *parseObj.plan)
	if err != nil {
		log.Fatal("couldn't run plan: ", err)
	}

	return inst.bufferID
}

//export octosql_get_records_count
func octosql_get_records_count(appID int32, parseID int32) int32 {
	return (*appResultBuffers)[(*appInstances)[appID].bufferID].GetRecordsCount()
}

//export octosql_get_record_fields_count
func octosql_get_record_fields_count(appID int32, parseID int32, recordID int32) int32 {
	return (*appResultBuffers)[(*appInstances)[appID].bufferID].GetRecordFieldsCount(recordID)
}

//export octosql_get_record_field_id
func octosql_get_record_field_id(appID int32, parseID int32, recordID int32, fieldName string) int32 {
	return (*appResultBuffers)[(*appInstances)[appID].bufferID].GetRecordFieldID(recordID, fieldName)
}

//export octosql_get_record_field_name
func octosql_get_record_field_name(appID int32, parseID int32, recordID int32, fieldID int32) *C.char {
	res := (*appResultBuffers)[(*appInstances)[appID].bufferID].GetRecordFieldName(recordID, fieldID)
	return C.CString(res)
}

//export octosql_get_record_field_type
func octosql_get_record_field_type(appID int32, parseID int32, recordID int32, fieldID int32) int32 {
	return (*appResultBuffers)[(*appInstances)[appID].bufferID].GetRecordFieldType(recordID, fieldID)
}

//export octosql_get_record_field_as_int
func octosql_get_record_field_as_int(appID int32, parseID int32, recordID int32, fieldID int32) int32 {
	return (*appResultBuffers)[(*appInstances)[appID].bufferID].GetRecordFieldAsInt(recordID, fieldID)
}

//export octosql_get_record_field_as_bool
func octosql_get_record_field_as_bool(appID int32, parseID int32, recordID int32, fieldID int32) bool {
	return (*appResultBuffers)[(*appInstances)[appID].bufferID].GetRecordFieldAsBool(recordID, fieldID)
}

//export octosql_get_record_field_as_float
func octosql_get_record_field_as_float(appID int32, parseID int32, recordID int32, fieldID int32) float64 {
	return (*appResultBuffers)[(*appInstances)[appID].bufferID].GetRecordFieldAsFloat(recordID, fieldID)
}

//export octosql_get_record_field_as_string
func octosql_get_record_field_as_string(appID int32, parseID int32, recordID int32, fieldID int32) *C.char {
	res := (*appResultBuffers)[(*appInstances)[appID].bufferID].GetRecordFieldAsString(recordID, fieldID)
	return C.CString(res)
}
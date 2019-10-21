package main

import (
	"github.com/cube2222/octosql/logical"
	"github.com/cube2222/octosql/storage/excel"
	"github.com/pkg/errors"
	"gopkg.in/yaml.v2"
	"log"
	"os"
	"context"
	"reflect"

	"github.com/cube2222/octosql/app"
	"github.com/cube2222/octosql/config"
	"github.com/cube2222/octosql/output"
	"github.com/cube2222/octosql/output/table"
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
var wasInited bool = false

type ParseInstance struct {
	ast* sqlparser.SelectStatement
	plan* logical.Node
}

type AppInstance struct {
	context context.Context
	sources *physical.DataSourceRepository
	config *config.Config
	app *app.App
	output output.Output
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

//export octosql_init
func octosql_init() (int32) {
	if wasInited {
		return 1
	}

	appInstances = &map[int32]*AppInstance{}
	appInstancesFI = 0

	appParseObjs = &map[int32]*ParseInstance{}
	appParseObjsFI = 0

	wasInited = true
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

	inst.output = table.NewOutput(os.Stdout, false)

	inst.app = app.NewApp(inst.config, inst.sources, inst.output)

	inst.context = context.Background()
	(*appInstances)[appInstancesFI] = &inst
	appInstancesFI++

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

//export octosql_run
func octosql_run(appID int32, parseID int32) {
	inst := (*appInstances)[appID]
	parseObj := (*appParseObjs)[parseID]

	err := inst.app.RunPlan(inst.context, *parseObj.plan)
	if err != nil {
		log.Fatal("couldn't run plan: ", err)
	}
}
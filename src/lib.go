package main

import (
	"github.com/cube2222/octosql/storage/excel"
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

func main () {

}

//export test
func test () {}

//export lolxd
func lolxd () {

	ctx := context.Background()
	cfg, err := config.ReadConfig("./octosql.yaml")

	dataSourceRespository, err := physical.CreateDataSourceRepositoryFromConfig(
		map[string]physical.Factory{
			"csv":      csv.NewDataSourceBuilderFactoryFromConfig,
			"json":     json.NewDataSourceBuilderFactoryFromConfig,
			"mysql":    mysql.NewDataSourceBuilderFactoryFromConfig,
			"postgres": postgres.NewDataSourceBuilderFactoryFromConfig,
			"redis":    redis.NewDataSourceBuilderFactoryFromConfig,
			"excel":    excel.NewDataSourceBuilderFactoryFromConfig,
		},
		cfg,
	)

	var out output.Output = table.NewOutput(os.Stdout, false)

	appInstance := app.NewApp(cfg, dataSourceRespository, out)

	// Parse query
	stmt, err := sqlparser.Parse("SELECT * from bikes bikes")
	if err != nil {
		log.Fatal("couldn't parse query: ", err)
	}
	typed, ok := stmt.(sqlparser.SelectStatement)
	if !ok {
		log.Fatalf("invalid statement type, wanted sqlparser.SelectStatement got %v", reflect.TypeOf(stmt))
	}
	plan, err := parser.ParseNode(typed)
	if err != nil {
		log.Fatal("couldn't parse query: ", err)
	}

	// Run query
	err = appInstance.RunPlan(ctx, plan)
	if err != nil {
		log.Fatal("couldn't run plan: ", err)
	}
}
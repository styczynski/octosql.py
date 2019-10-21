#!/bin/bash

echo "Mount: [$(pwd)]"

docker run --rm -it -v "$(pwd)":/go/github.com/styczynski/octosql.py -w /go/github.com/styczynski/octosql.py dockercore/golang-cross:1.12.10 sh -c '
	export GOPATH=/go
	#go get "github.com/cube2222/octosql"

	#the full list of the platforms: https://golang.org/doc/install/source#environment
  platforms="darwin-amd64-1"

  for platform in "$platforms"
  do
      export GOOS=$(echo $platform | cut -f1 -d-)
      export GOARCH=$(echo $platform | cut -f2 -d-)
      export CGO_ENABLED=$(echo $platform | cut -f3 -d-)
      echo "Building $GOOS-$GOARCH"

      go build -o build/$GOOS-$GOARCH/octosql -buildmode=c-shared ./src/lib.go
      if [ $? -ne 0 ]; then
          echo "An error has occurred! Aborting the script execution..."
          exit 1
      fi
  done
    '
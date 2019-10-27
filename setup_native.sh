#!/bin/bash

cd src/custom_storage && echo "HERE -> " && echo $(pwd) && make all && cd ../..
eval "$(curl -sL https://raw.githubusercontent.com/travis-ci/gimme/master/gimme | GIMME_GO_VERSION=1.13 bash)" && go install ./... && go build -o "$1" -buildmode=c-archive "$2"
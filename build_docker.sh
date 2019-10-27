#!/bin/bash

VERCMD="$(cat setup.cfg | grep 'current_version = ')"
VERCMD=${VERCMD//[[:blank:]]/}
eval "export $VERCMD"

echo "Will create docker image with version label: ${current_version}"
if [ -z "$current_version" ]
then
      echo "Failed to get the project version. It may not be installed. Please run make to build the project first."
      exit 1
else
      echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin
      docker build -t octosql-py .
      docker tag octosql-py styczynski/octosql-py:${current_version}
      docker push styczynski/octosql-py:${current_version}
fi
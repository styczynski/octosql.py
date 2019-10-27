#!/bin/bash

echo "Will create docker image with version label: ${OCTOSQL_CURRENT_VERSION}"
if [ -z "$OCTOSQL_CURRENT_VERSION" ]
then
      echo "Failed to get the project version. It may not be installed. Please run make to build the project first."
      exit 1
else
      echo "$DOCKER_PASSWORD" | docker login -u "$DOCKER_USERNAME" --password-stdin
      docker build -t octosql-py .
      docker tag octosql-py styczynski/octosql-py:${OCTOSQL_CURRENT_VERSION}
      docker push styczynski/octosql-py:${OCTOSQL_CURRENT_VERSION}
fi
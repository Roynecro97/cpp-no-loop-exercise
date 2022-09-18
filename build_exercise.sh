#!/usr/bin/env bash

set -euo pipefail

cd "$(dirname "$BASH_SOURCE")"

EXERCISE_IMAGE=cpp-ex
BUILDER_IMAGE=${EXERCISE_IMAGE}-builder

docker build -t $BUILDER_IMAGE -f plugin/builder.dockerfile plugin
docker run --rm -t -v "$(pwd):$(pwd)" -w "$(pwd)" $BUILDER_IMAGE ./plugin/update-plugin.sh
docker build -t $EXERCISE_IMAGE docker

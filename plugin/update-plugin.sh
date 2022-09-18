#!/usr/bin/env bash

cd "$(dirname "$BASH_SOURCE")"

BUILDER="$(readlink -f ./build.sh)"
DEST="$(readlink -f ../docker/plugin.so)"

FILES="$DEST src/*.cpp $BUILDER $(basename "$BASH_SOURCE")"

if [[ -f "$DEST" ]]; then
    echo -n newest:' '
    stat -c '%Y %n' $FILES | sort -n | tail -1 | awk '{ print $2 }'
else
    echo missing: $DEST
fi

if [[ ! -f "$DEST" || $(stat -c '%Y %n' $FILES | sort -n | tail -1 | awk '{ print $2 }') != "$DEST" ]]; then
    (
        cd ./src
        time $BUILDER -o $DEST *.cpp
    )
fi

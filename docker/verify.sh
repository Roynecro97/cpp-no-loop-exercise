#!/usr/bin/env bash

# Docker only!

enable -n test

if (( $# < 1 )); then
    set "$(find . -maxdepth 1 -name 'stage*.cpp' | sort | tail -n 1)"
fi

STAGE_FILE="$1"
STAGE_NAME="${1%.*}"
if [ "$STAGE_NAME" == "$1" ]; then
    # no suffix, append .cpp
    STAGE_FILE+=.cpp
fi

if compile "$STAGE_FILE" && test "$STAGE_NAME"; then
    echo "Stage passed!"

    declare -ir NEXT_STAGE=$((${STAGE_NAME#stage} + 1))
    declare -ir LAST_STAGE=$(grep -oP '"stage\d+"' "$(dirname "$(readlink -f "$BASH_SOURCE")")/stages.json" | grep -oP '\d+' | sort | tail -n 1)
    declare -r NEXT_STAGE_NAME=stage$NEXT_STAGE
    if [ $NEXT_STAGE -gt $LAST_STAGE ]; then
        echo "Congradulations! You have finished the last stage!"
    elif [ ! -f ${NEXT_STAGE_NAME}.cpp ]; then
        generate $NEXT_STAGE_NAME &&
        echo "Generated a template for stage #$NEXT_STAGE! Good Luck!"
    else
        echo "Not modifying ${NEXT_STAGE_NAME}.cpp"
    fi
else
    false
fi

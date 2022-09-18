#!/usr/bin/env bash

# Docker only!

declare -r FIRST_STAGE=$(grep -oP '(?<=")stage\d+(?=")' "$(dirname "$(readlink -f "$BASH_SOURCE")")/stages.json" | sort | head -n 1)

if [ ! -f $FIRST_STAGE.cpp ]; then
    generate $FIRST_STAGE &&
    echo "Generated a template for stage #${FIRST_STAGE#stage}! Good Luck!"
else
    echo "Not modifying $FIRST_STAGE.cpp"
fi

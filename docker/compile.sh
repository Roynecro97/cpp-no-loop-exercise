#!/usr/bin/env bash

clang++-10 -Xclang -load -Xclang exercise-rules.so \
           -Xclang -add-plugin -Xclang no-loops \
           -Xclang -add-plugin -Xclang no-recursion \
           -Xclang -add-plugin -Xclang no-asm \
           -Xclang -add-plugin -Xclang warnings \
           -std=gnu++14 -Wall -Wextra -Wpedantic -Wshadow -Wconversion --pedantic-errors "$1" -o "${1%.cpp}"

FROM ubuntu:20.04

RUN apt update && apt install -y ca-certificates lsb-release wget software-properties-common gnupg && rm -rf /var/lib/apt/lists/*

RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN add-apt-repository 'deb http://apt.llvm.org/focal/  llvm-toolchain-focal-10 main'

RUN apt update && apt install -y clang-10 llvm-10-dev libclang-common-10-dev libclang-10-dev libclang-cpp10-dev && rm -rf /var/lib/apt/lists/* && \
ln -sf /bin/bash /bin/sh

ENV force_color_prompt=yes

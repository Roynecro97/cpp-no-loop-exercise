FROM ubuntu:20.04

ENV SCRIPTS_PATH /usr/local/bin

RUN apt update && apt install -y ca-certificates lsb-release wget software-properties-common gnupg && rm -rf /var/lib/apt/lists/*

RUN wget -O - https://apt.llvm.org/llvm-snapshot.gpg.key | apt-key add -
RUN add-apt-repository 'deb http://apt.llvm.org/focal/  llvm-toolchain-focal-10 main'

RUN apt update && apt install -y clang-10 python3-pip rename && rm -rf /var/lib/apt/lists/* && \
pip3 install cheetah3 && \
ln -sf /bin/bash /bin/sh

COPY plugin.so /usr/lib/exercise-rules.so
COPY compile.sh generate.py list.py main.cpp.tmpl stages.json start.sh test.py verify.sh ${SCRIPTS_PATH}/
RUN rename 's/.(sh|py)//' ${SCRIPTS_PATH}/*.*

ENV force_color_prompt=yes

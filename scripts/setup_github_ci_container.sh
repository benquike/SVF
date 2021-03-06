#!/bin/bash

sudo apt install -y clang-10 clang-tools-10
sudo apt install -y libboost-serialization-dev libboost-program-options-dev
sudo apt install -y libboost-system-dev libboost-filesystem-dev
sudo update-alternatives --install /usr/bin/clang clang /usr/bin/clang-10 40  \
    --slave /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-10 \
    --slave /usr/bin/opt opt /usr/bin/opt-10 \
    --slave /usr/bin/llvm-as llvm-as /usr/bin/llvm-as-10

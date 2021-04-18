FROM ubuntu:20.04

RUN echo "deb http://archive.ubuntu.com/ubuntu/ focal main restricted universe multiverse" > /etc/apt/sources.list \
    && echo "deb-src http://archive.ubuntu.com/ubuntu/ focal main restricted universe multiverse" >> /etc/apt/sources.list \
    && echo "deb http://archive.ubuntu.com/ubuntu/ focal-updates main restricted universe multiverse" >> /etc/apt/sources.list \
    && echo "deb-src http://archive.ubuntu.com/ubuntu/ focal-updates main restricted universe multiverse" >> /etc/apt/sources.list \
    && echo "deb http://archive.ubuntu.com/ubuntu/ focal-security main restricted universe multiverse" >> /etc/apt/sources.list \
    && echo "deb-src http://archive.ubuntu.com/ubuntu/ focal-security main restricted universe multiverse" >> /etc/apt/sources.list \
    && echo "deb http://archive.canonical.com/ubuntu focal partner" >> /etc/apt/sources.list \
    && echo "deb-src http://archive.canonical.com/ubuntu focal partner" >> /etc/apt/sources.list

RUN ln -fs /usr/share/zoneinfo/US/Eastern /etc/localtime

RUN apt-get update && apt-get install -y software-properties-common apt-utils \
    locales build-essential binutils gcc \
    make wget vim clang-10 \
    clang-tidy-10 llvm-10-dev clang-format-10 clang-tools-10 \
    cmake libz3-dev git curl unzip \
    apt-transport-https ca-certificates gnupg \
    python3-pip \
    libboost-all-dev

RUN pip3 install cmakelang

RUN rm -rf /var/lib/apt/lists/*

RUN update-alternatives --install /usr/bin/clang clang /usr/bin/clang-10 40  \
    --slave /usr/bin/clang++ clang++ /usr/bin/clang++-10  \
    --slave /usr/bin/llvm-config llvm-config /usr/bin/llvm-config-10 \
    --slave /usr/bin/opt opt /usr/bin/opt-10 \
    --slave /usr/bin/llvm-as llvm-as /usr/bin/llvm-as-10 \
    --slave /usr/bin/llvm-symbolizer llvm-symbolizer /usr/bin/llvm-symbolizer-10 \
    --slave /usr/bin/clang-format clang-format /usr/bin/clang-format-10

RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 40

RUN useradd -ms /bin/bash test

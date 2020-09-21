FROM ubuntu:16.04

run apt update
run apt install gcc-5 -y
run apt install wget vim -y
run apt install make cmake git subversion -y
run apt install clang-6.0 llvm-6.0 -y
run apt install libgmp-dev libmpfr-dev -y

run mkdir -p /home/abs_interp
run mkdir -p /home/apron/mp
run wget http://apron.cri.ensmp.fr/library/apron-0.9.10.tgz -O /home/apron/apron-0.9.10.tgz
run cd /home/apron/ && tar -xf apron-0.9.10.tgz && cd apron-0.9.10 && mv Makefile.config.model Makefile.config 
COPY strdup_fix.patch /home/apron/apron-0.9.10/
run cd /home/apron/apron-0.9.10 && (patch -p1 < strdup_fix.patch) && make c && (make install || true)
run groupadd -g 1000 divyanjali
run useradd -m -u 1000 -g 1000 -s /bin/bash divyanjali
run export CPLUS_INCLUDE_PATH=/tmp/include/
run export LD_LIBRARY_PATH=/tmp/lib/
run ln -s /usr/bin/clang-6.0  /usr/bin/clang
run ln -s /usr/bin/clang++-6.0  /usr/bin/clang++
run ln -s /usr/bin/opt-6.0  /usr/bin/opt


ENTRYPOINT bash


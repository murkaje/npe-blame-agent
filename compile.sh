#!/bin/bash

mkdir -p target
pushd target

clang++ -pthread -m64 -c -g -fPIC -D_LP64=1 -std=c++14 -I$JAVA_HOME/include -I$JAVA_HOME/include/linux -I../src/main/include \
../src/main/c/onLoad.cpp ../src/main/c/exceptionCallback.cpp ../src/main/c/logging.cpp ../src/main/c/util.cpp

clang++ -z defs -static-libgcc -g -m64 -shared -o libexceptioninfo64.so onLoad.o exceptionCallback.o logging.o util.o

popd
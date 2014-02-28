#!/bin/bash

# RUN THIS SCRIPT FROM THE PROJECT'S ROOT LEVEL DIRECTORY

INSTALL_DIR=pawned-1.0

mkdir -p release_build
cd release_build
mkdir binaries

mkdir -p win64
cd win64
cmake ../.. -DTARGET_SUFFIX="win64" -DCMAKE_TOOLCHAIN_FILE=../../cmake/win64.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../${INSTALL_DIR}
make install
cd ..

mkdir -p win32
cd win32
cmake ../.. -DTARGET_SUFFIX="win32" -DCMAKE_TOOLCHAIN_FILE=../../cmake/win32.cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../${INSTALL_DIR}
make install
cd ..

mkdir -p linux64
cd linux64
cmake ../.. -DTARGET_SUFFIX="linux64" -DCMAKE_C_FLAGS="-m64" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../${INSTALL_DIR}
make install
cd ..

mkdir -p linux32
cd linux32
cmake ../.. -DTARGET_SUFFIX="linux32" -DCMAKE_C_FLAGS="-m32" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../${INSTALL_DIR}
make install
cd ..

zip ${INSTALL_DIR}.zip -r ${INSTALL_DIR}

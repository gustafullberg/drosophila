#!/bin/bash

# RUN THIS SCRIPT FROM THE PROJECT'S ROOT LEVEL DIRECTORY

INSTALL_DIR=drosophila-1.6

mkdir -p release_build
cd release_build
mkdir binaries

mkdir -p win64
cd win64
cmake ../.. -DTARGET_SUFFIX="win64" -DCMAKE_TOOLCHAIN_FILE=../../cmake/win64.cmake -DCMAKE_C_FLAGS="-mpopcnt -mlzcnt" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../${INSTALL_DIR}
make install
cd ..

mkdir -p linux64
cd linux64
cmake ../.. -DTARGET_SUFFIX="linux64" -DCMAKE_C_FLAGS="-m64 -mpopcnt -mlzcnt" -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=../${INSTALL_DIR}
make install
cd ..

zip ${INSTALL_DIR}.zip -r ${INSTALL_DIR}

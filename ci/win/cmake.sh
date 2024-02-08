#!/bin/bash

where cl.exe
export CC=cl.exe
export CXX=cl.exe

cmake -G "Ninja" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX="$CONDA_PREFIX" \
    -DWITH_TESTS=ON \
    -DCMAKE_VERBOSE_MAKEFILE=OFF \
    -DOPENSSL_ROOT_DIR="$CONDA_PREFIX/Library" \
    -DCMAKE_PREFIX_PATH="$CONDA_PREFIX/Library" \
    ..

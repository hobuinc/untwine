#!/bin/bash

where cl.exe
export CC=cl.exe
export CXX=cl.exe


git clone https://github.com/hobu/laz-perf.git laz-perf && cd laz-perf && \
    mkdir build && cd build && \
    cmake -G Ninja .. -DWITH_TESTS=ON -DCMAKE_BUILD_TYPE=RelWithDebInfo  -Dgtest_force_shared_crt=ON -DCMAKE_INSTALL_PREFIX=$CONDA_PREFIX/Library && \
    ninja install && \
    cd ../..

cmake -G "Ninja" \
    -DCMAKE_BUILD_TYPE=RelWithDebInfo \
    -DCMAKE_INSTALL_PREFIX="$CONDA_PREFIX" \
    -DWITH_TESTS=ON \
    -DCMAKE_VERBOSE_MAKEFILE=OFF \
    -DOPENSSL_ROOT_DIR="$CONDA_PREFIX/Library" \
    -DCMAKE_PREFIX_PATH="$CONDA_PREFIX/Library" \
    ..
